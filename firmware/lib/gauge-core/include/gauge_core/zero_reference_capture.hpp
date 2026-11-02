#pragma once

#include <cstddef>
#include <cstdint>

namespace gauge_core {

/// Hard memory bound for one atmospheric-reference capture request.
/// The bound limits stack/work requirements on the ESP32 and makes an
/// accidentally oversized request fail explicitly instead of using unbounded
/// memory. It is a software safety bound, not a recommended capture duration.
constexpr std::size_t kMaximumZeroCaptureSamples = 64;

/// Acceptance limits supplied by the caller from a sensor profile and tests.
///
/// Capturing zero means establishing the voltage that represents atmospheric
/// pressure for this installation. This can compensate a small sensor offset,
/// but it must be an explicit operation with the pressure port actually at
/// atmosphere. The capture helper cannot know whether the engine is off, the
/// port is open, or the sensor has warmed up; the future settings flow must
/// guide the user and enforce the project's chosen procedure. No physical
/// acceptance limits are guessed by this hardware-independent type.
struct ZeroCaptureCriteria {
  std::uint32_t minimumVoltageMicrovolts = 0;
  std::uint32_t maximumVoltageMicrovolts = 0;
  std::uint32_t maximumPeakToPeakMicrovolts = 0;
  std::size_t minimumSampleCount = 0;

  /// Validate the voltage interval and bounded sample-count requirement.
  /// @return True when a capture can be evaluated safely.
  bool isValid() const;
};

/// Explains why the requested reference capture was or was not accepted.
enum class ZeroCaptureStatus : std::uint8_t {
  kAccepted,
  kInvalidCriteria,
  kInsufficientSamples,
  kTooManySamples,
  kVoltageOutOfRange,
  kUnstable
};

/// Summary returned by a pure, deterministic zero-reference evaluation.
struct ZeroCaptureResult {
  ZeroCaptureStatus status = ZeroCaptureStatus::kInvalidCriteria;
  std::uint32_t zeroVoltageMicrovolts = 0;
  std::uint32_t minimumObservedMicrovolts = 0;
  std::uint32_t maximumObservedMicrovolts = 0;
};

/// Evaluate a bounded set of voltage samples for an explicitly requested zero.
/// This class only calculates a candidate reference. It does not read the ADC,
/// save settings to flash/NVS, or trigger automatic zeroing at startup.
class ZeroReferenceCapture {
 public:
  /// Accept a stable in-range window and return its rounded arithmetic mean.
  /// The min/max spread check rejects noisy or changing pressure conditions;
  /// averaging then reduces small random ADC variation in a steady window.
  /// @param samples ADC-derived signal voltages in microvolts.
  /// @param sampleCount Number of values in the array.
  /// @param criteria Measured/profile-derived acceptance criteria.
  /// @return Capture status, observed extrema, and accepted mean if valid.
  ZeroCaptureResult evaluate(const std::uint32_t* samples,
                             std::size_t sampleCount,
                             const ZeroCaptureCriteria& criteria) const;
};

}  // namespace gauge_core
