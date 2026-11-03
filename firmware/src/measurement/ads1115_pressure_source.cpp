#include "ads1115_pressure_source.hpp"

namespace ls_pcv {
namespace measurement {
namespace {

// TI documents that offset can produce a negative single-ended code near 0 V.
// The bench observed exactly -1 code while A0 was grounded. At the selected
// +/-6.144 V PGA range, one code is 187.5 uV, so treat this single boundary
// code as 0 V while preserving the original signed code in the snapshot.
// Larger negative codes remain faults: they are outside the intended A0-to-GND
// input range and must not be silently converted into plausible voltages.
constexpr std::int16_t kSingleEndedZeroOffsetToleranceCodes = 1;

Ads1115Result mapBusFailure(I2cTransactionResult result) {
  return result == I2cTransactionResult::kTimeout
      ? Ads1115Result::kI2cTimeout : Ads1115Result::kI2cError;
}

}  // namespace

/// Compose the bus driver with the tested pressure math and history tracker.
Ads1115PressureSource::Ads1115PressureSource(
    I2cRegisterBus& bus, std::uint8_t address,
    gauge_core::PressureCalibrationProfile profile,
    gauge_core::PressureBandThresholds thresholds)
    : bus_(bus), device_(bus, address), profile_(profile), tracker_(thresholds) {}

/// Perform one bounded step; callers schedule this from the acquisition task.
bool Ads1115PressureSource::poll(std::uint32_t nowMs) {
  bool published = false;

  if (!initialized_) {
    const bool retryDue = !initializationAttempted_ ||
        static_cast<std::uint32_t>(nowMs - lastInitializationAttemptMs_) >=
            kInitializationRetryMs;
    if (!retryDue) {
      (void)tracker_.sampleAt(nowMs);
      return false;
    }

    // First start the physical I2C controller, then probe the ADC register.
    // Keep these failures distinguishable from a completed bad voltage read.
    initializationAttempted_ = true;
    lastInitializationAttemptMs_ = nowMs;
    I2cTransactionResult busResult = bus_.begin();
    if (busResult == I2cTransactionResult::kSuccess) {
      lastDeviceResult_ = device_.initialize();
    } else {
      lastDeviceResult_ = mapBusFailure(busResult);
    }
    if (!isSuccessfulBusResult(lastDeviceResult_)) {
      recordBusFault(nowMs, lastDeviceResult_);
      (void)tracker_.sampleAt(nowMs);
      return true;
    }
    initialized_ = true;
    conversionState_ = ConversionState::kIdle;
    // Initialization is one bounded probe transaction. Start conversion on a
    // later poll so an acquisition-task turn never chains I2C transactions.
    (void)tracker_.sampleAt(nowMs);
    return false;
  }

  if (conversionState_ == ConversionState::kIdle) {
    lastDeviceResult_ = device_.startSingleShotA0();
    if (lastDeviceResult_ != Ads1115Result::kSuccess) {
      initialized_ = false;
      conversionState_ = ConversionState::kIdle;
      recordBusFault(nowMs, lastDeviceResult_);
      (void)tracker_.sampleAt(nowMs);
      return true;
    }
    conversionState_ = ConversionState::kWaiting;
    conversionStartedAtMs_ = nowMs;
    (void)tracker_.sampleAt(nowMs);
    return false;
  }

  if (conversionState_ == ConversionState::kReadResult) {
    return readReadyConversion(nowMs);
  }

  // OS is the ADS1115's conversion-ready bit. Polling it once per task turn
  // avoids tying up the CPU for the nominal 7.8 ms conversion interval.
  bool ready = false;
  lastDeviceResult_ = device_.conversionReady(ready);
  if (lastDeviceResult_ == Ads1115Result::kNotReady) {
    if (static_cast<std::uint32_t>(nowMs - conversionStartedAtMs_) >=
        kConversionTimeoutMs) {
      conversionState_ = ConversionState::kIdle;
      lastDeviceResult_ = Ads1115Result::kConversionTimeout;
      recordAdcFault(nowMs, lastDeviceResult_);
      published = true;
    }
    (void)tracker_.sampleAt(nowMs);
    return published;
  }
  if (lastDeviceResult_ != Ads1115Result::kSuccess || !ready) {
    initialized_ = false;
    conversionState_ = ConversionState::kIdle;
    recordBusFault(nowMs, lastDeviceResult_);
    (void)tracker_.sampleAt(nowMs);
    return true;
  }

  // Defer conversion-register access to the next task turn: readiness polling
  // and data retrieval are two separate I2C operations with separate results.
  conversionState_ = ConversionState::kReadResult;
  (void)tracker_.sampleAt(nowMs);
  return false;
}

/// Read and process the result only after a previous poll observed OS=1.
bool Ads1115PressureSource::readReadyConversion(std::uint32_t nowMs) {
  std::int16_t rawCode = 0;
  lastDeviceResult_ = device_.readConversion(rawCode);
  conversionState_ = ConversionState::kIdle;
  if (lastDeviceResult_ != Ads1115Result::kSuccess) {
    initialized_ = false;
    recordBusFault(nowMs, lastDeviceResult_);
    (void)tracker_.sampleAt(nowMs);
    return true;
  }

  const bool published = recordConversion(nowMs, rawCode);
  (void)tracker_.sampleAt(nowMs);
  return published;
}

/// Copy only after the tracker has processed the latest completed conversion.
gauge_core::MeasurementSnapshot Ads1115PressureSource::snapshot() const {
  return tracker_.snapshot();
}

/// Publish a transport failure and suppress the current numeric pressure.
void Ads1115PressureSource::recordBusFault(std::uint32_t nowMs,
                                          Ads1115Result result) {
  lastDeviceResult_ = result;
  gauge_core::MeasurementObservation observation;
  observation.source = gauge_core::MeasurementSource::kLive;
  observation.quality = gauge_core::MeasurementQuality::kI2cError;
  observation.acquiredAtMs = nowMs;
  (void)tracker_.record(observation);
}

/// Publish a conversion/input failure without treating its code as pressure.
void Ads1115PressureSource::recordAdcFault(std::uint32_t nowMs,
                                          Ads1115Result result,
                                          bool hasRawCode,
                                          std::int16_t rawCode) {
  lastDeviceResult_ = result;
  gauge_core::MeasurementObservation observation;
  observation.source = gauge_core::MeasurementSource::kLive;
  observation.quality = gauge_core::MeasurementQuality::kAdcError;
  observation.acquiredAtMs = nowMs;
  observation.hasRawAdcCode = hasRawCode;
  observation.rawAdcCode = rawCode;
  (void)tracker_.record(observation);
}

/// Convert the raw code and update the tracker before exposing a new snapshot.
bool Ads1115PressureSource::recordConversion(std::uint32_t nowMs,
                                            std::int16_t rawCode) {
  if (rawCode < -kSingleEndedZeroOffsetToleranceCodes) {
    lastDeviceResult_ = Ads1115Result::kInvalidSingleEndedCode;
    recordAdcFault(nowMs, lastDeviceResult_, true, rawCode);
    return true;
  }

  const std::int32_t signedSignalMicrovolts =
      Ads1115Device::codeToMicrovolts(rawCode);
  // Clamp only the documented/measured one-code zero-boundary result. Keep
  // rawCode unchanged below so diagnostics can still show the actual ADC code.
  const std::uint32_t nonnegativeMicrovolts = signedSignalMicrovolts < 0
      ? 0u : static_cast<std::uint32_t>(signedSignalMicrovolts);
  const gauge_core::PressureConversionResult converted =
      gauge_core::PressureConverter().convert(nonnegativeMicrovolts, profile_);
  gauge_core::MeasurementObservation observation;
  observation.source = gauge_core::MeasurementSource::kLive;
  observation.acquiredAtMs = nowMs;
  observation.hasRawAdcCode = true;
  observation.rawAdcCode = rawCode;
  observation.hasSignalVoltage = true;
  observation.signalVoltageMicrovolts = nonnegativeMicrovolts;

  if (converted.status == gauge_core::PressureConversionStatus::kOutOfDomain) {
    observation.quality = gauge_core::MeasurementQuality::kOutOfDomain;
    (void)tracker_.record(observation);
    return true;
  }
  if (converted.status != gauge_core::PressureConversionStatus::kValid) {
    lastDeviceResult_ = Ads1115Result::kPressureConversionError;
    recordAdcFault(nowMs, lastDeviceResult_, true, rawCode);
    return true;
  }

  observation.quality = gauge_core::MeasurementQuality::kValid;
  observation.hasPressure = true;
  observation.pressure = converted.pressure;
  const bool accepted = tracker_.record(observation);
  if (accepted) lastDeviceResult_ = Ads1115Result::kSuccess;
  return accepted;
}

/// True only for transport operations that completed successfully.
bool Ads1115PressureSource::isSuccessfulBusResult(Ads1115Result result) const {
  return result == Ads1115Result::kSuccess;
}

}  // namespace measurement
}  // namespace ls_pcv
