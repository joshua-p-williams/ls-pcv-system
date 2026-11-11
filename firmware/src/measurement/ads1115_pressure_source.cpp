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

/// The bench showed one-code within-session variation and at most six codes
/// between the two atmospheric sessions (about 1.125 mV). A 2 mV peak-to-peak
/// allowance leaves modest headroom while rejecting a changing pressure.
/// The 1.3-1.7 V plausibility interval comes from the documented family
/// atmospheric-output range. Sixty-four consecutive conversions bound capture
/// work and average the observed single-code quantization variation.
gauge_core::ZeroCaptureCriteria zeroCaptureCriteria() {
  gauge_core::ZeroCaptureCriteria criteria;
  criteria.minimumVoltageMicrovolts = 1300000;
  criteria.maximumVoltageMicrovolts = 1700000;
  criteria.maximumPeakToPeakMicrovolts = 2000;
  criteria.minimumSampleCount = 64;
  return criteria;
}

Ads1115Result mapBusFailure(I2cTransactionResult result) {
  return result == I2cTransactionResult::kTimeout
      ? Ads1115Result::kI2cTimeout : Ads1115Result::kI2cError;
}

}  // namespace

/// Compose the bus driver with the tested pressure math and history tracker.
Ads1115PressureSource::Ads1115PressureSource(
    I2cRegisterBus& bus, std::uint8_t address,
    gauge_core::PressureCalibrationProfile profile,
    gauge_core::PressureBandThresholds thresholds,
    gauge_core::ZeroReferenceStore* zeroStore)
    : bus_(bus), device_(bus, address), profile_(profile), tracker_(thresholds),
      zeroStore_(zeroStore),
      zeroManager_(profile.zeroSignalMicrovolts, zeroCaptureCriteria()) {}

/// Perform one bounded step; callers schedule this from the acquisition task.
bool Ads1115PressureSource::poll(std::uint32_t nowMs) {
  bool published = false;

  // Preferences is initialized lazily from this acquisition context, not from
  // global constructors or the LVGL/application task.
  initializeZeroReference();

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
  gauge_core::MeasurementSnapshot result = tracker_.snapshot();
  result.zeroReference = zeroManager_.state();
  result.zeroCaptureSamples = static_cast<std::uint8_t>(zeroSampleCount_);
  result.zeroReferenceMicrovolts = zeroManager_.activeZeroMicrovolts();
  result.zeroCaptureStatus = zeroManager_.lastCaptureStatus();
  return result;
}

/// Start collection only when a persistent store is available and no capture runs.
bool Ads1115PressureSource::beginZeroCapture() {
  initializeZeroReference();
  if (zeroStore_ == nullptr || zeroCaptureActive_ ||
      !zeroCaptureCriteria().isValid()) {
    return false;
  }
  zeroSampleCount_ = 0;
  zeroCaptureActive_ = true;
  zeroManager_.beginCapture();
  return true;
}

/// Load persistent state once and synchronize the voltage used by conversion.
/// The manager and display expose the loaded value, but pressure math uses the
/// profile; both must receive the same reference after a power cycle.
void Ads1115PressureSource::initializeZeroReference() {
  if (zeroInitialized_) return;
  zeroInitialized_ = true;
  if (zeroStore_ != nullptr) (void)zeroManager_.initialize(*zeroStore_);
  profile_.zeroSignalMicrovolts = zeroManager_.activeZeroMicrovolts();
}

/// Discard collected samples while preserving the active and saved reference.
void Ads1115PressureSource::cancelZeroCapture() {
  if (!zeroCaptureActive_) return;
  zeroCaptureActive_ = false;
  zeroSampleCount_ = 0;
  zeroManager_.cancelCapture();
}

/// Publish a transport failure and suppress the current numeric pressure.
void Ads1115PressureSource::recordBusFault(std::uint32_t nowMs,
                                          Ads1115Result result) {
  rejectInterruptedZeroCapture();
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
  rejectInterruptedZeroCapture();
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
  const bool captureFinished = collectZeroSample(nonnegativeMicrovolts);
  const bool referenceChanged = captureFinished &&
      zeroManager_.state() == gauge_core::ZeroReferenceState::kSavedSuccessfully;
  if (referenceChanged) {
    // Existing history/extrema were calculated using the old reference. Begin
    // a fresh run so values from two different zero settings are never mixed.
    profile_.zeroSignalMicrovolts = zeroManager_.activeZeroMicrovolts();
    tracker_.reset();
  }
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

/// Add one fresh ADC result and save only after the complete stable window passes.
bool Ads1115PressureSource::collectZeroSample(std::uint32_t voltageMicrovolts) {
  if (!zeroCaptureActive_) return false;
  if (zeroSampleCount_ < zeroSamples_.size()) {
    zeroSamples_[zeroSampleCount_++] = voltageMicrovolts;
  }
  if (zeroSampleCount_ < zeroManager_.requiredSampleCount()) return false;

  (void)zeroManager_.finishCapture(
      zeroSamples_.data(), zeroSampleCount_, *zeroStore_);
  zeroCaptureActive_ = false;
  return true;
}

/// A lost acquisition makes the requested consecutive window unsuitable.
void Ads1115PressureSource::rejectInterruptedZeroCapture() {
  if (!zeroCaptureActive_ || zeroStore_ == nullptr) return;
  (void)zeroManager_.finishCapture(
      zeroSamples_.data(), zeroSampleCount_, *zeroStore_);
  zeroCaptureActive_ = false;
}

}  // namespace measurement
}  // namespace ls_pcv
