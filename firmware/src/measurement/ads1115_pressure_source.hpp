#pragma once

#include <array>
#include <cstdint>

#include "gauge_core/measurement_tracker.hpp"
#include "gauge_core/pressure_converter.hpp"
#include "gauge_core/zero_reference_capture.hpp"
#include "ads1115_device.hpp"

namespace ls_pcv {
namespace measurement {

/// Owns the ADS1115 read/conversion pipeline and publishes coherent snapshots.
///
/// Call poll() repeatedly from the dedicated acquisition context. Each call
/// performs at most one bounded I2C transaction; conversion readiness is polled
/// across calls rather than waiting in a busy loop. Arduino Wire itself is
/// synchronous, so the board adapter's finite transaction timeout bounds how
/// long one call can occupy its task. This keeps conversion waits independent
/// of LVGL work and makes failures visible in MeasurementQuality.
class Ads1115PressureSource {
 public:
  /// Time budget exceeds the nominal 7.8 ms conversion at 128 SPS and allows
  /// ordinary scheduling jitter without waiting indefinitely.
  static constexpr std::uint32_t kConversionTimeoutMs = 25;
  /// A missing module is retried slowly so a disconnected bus is not hammered.
  static constexpr std::uint32_t kInitializationRetryMs = 1000;

  Ads1115PressureSource(I2cRegisterBus& bus, std::uint8_t address,
                        gauge_core::PressureCalibrationProfile profile = {},
                        gauge_core::PressureBandThresholds thresholds = {},
                        gauge_core::ZeroReferenceStore* zeroStore = nullptr);

  /// Advance initialization/conversion state without waiting for conversion.
  /// @param nowMs Monotonic millisecond time supplied by the board task.
  /// @return True when a new measurement or fault state was recorded.
  bool poll(std::uint32_t nowMs);

  /// Copy the latest state after poll has recorded any completed conversion.
  /// This is the publication boundary; callers may safely send this value to
  /// the application/UI without exposing the tracker's mutable internal state.
  gauge_core::MeasurementSnapshot snapshot() const;

  /// Begin the bounded capture after the application has explicitly confirmed it.
  bool beginZeroCapture();
  /// Cancel a pending capture without altering the prior saved reference.
  void cancelZeroCapture();

  /// Last low-level result, useful for a later diagnostic screen or log.
  Ads1115Result lastDeviceResult() const { return lastDeviceResult_; }

 private:
  enum class ConversionState : std::uint8_t { kIdle, kWaiting, kReadResult };

  void recordBusFault(std::uint32_t nowMs, Ads1115Result result);
  void recordAdcFault(std::uint32_t nowMs, Ads1115Result result,
                     bool hasRawCode = false, std::int16_t rawCode = 0);
  bool readReadyConversion(std::uint32_t nowMs);
  void initializeZeroReference();
  bool recordConversion(std::uint32_t nowMs, std::int16_t rawCode);
  bool isSuccessfulBusResult(Ads1115Result result) const;
  bool collectZeroSample(std::uint32_t voltageMicrovolts);
  void rejectInterruptedZeroCapture();

  I2cRegisterBus& bus_;
  Ads1115Device device_;
  gauge_core::PressureCalibrationProfile profile_;
  gauge_core::MeasurementTracker tracker_;
  gauge_core::ZeroReferenceStore* zeroStore_;
  gauge_core::ZeroReferenceManager zeroManager_;
  std::array<std::uint32_t, gauge_core::kMaximumZeroCaptureSamples>
      zeroSamples_{};
  std::size_t zeroSampleCount_ = 0;
  bool zeroInitialized_ = false;
  bool zeroCaptureActive_ = false;
  Ads1115Result lastDeviceResult_ = Ads1115Result::kNotInitialized;
  ConversionState conversionState_ = ConversionState::kIdle;
  bool initialized_ = false;
  bool initializationAttempted_ = false;
  std::uint32_t lastInitializationAttemptMs_ = 0;
  std::uint32_t conversionStartedAtMs_ = 0;
};

}  // namespace measurement
}  // namespace ls_pcv
