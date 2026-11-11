#pragma once

#include <cstddef>
#include <cstdint>

#include "gauge_types.hpp"

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

/// Summary returned by a pure, deterministic zero-reference evaluation.
struct ZeroCaptureResult {
  ZeroCaptureStatus status = ZeroCaptureStatus::kInvalidCriteria;
  std::uint32_t zeroVoltageMicrovolts = 0;
  std::uint32_t minimumObservedMicrovolts = 0;
  std::uint32_t maximumObservedMicrovolts = 0;
};

/// Versioned value stored by the platform adapter in nonvolatile settings.
struct ZeroReferenceRecord {
  static constexpr std::uint16_t kCurrentVersion = 1;
  std::uint16_t version = kCurrentVersion;
  std::uint32_t zeroVoltageMicrovolts = 0;
};

/// Distinguish a missing first-use setting from damaged data or a storage error.
enum class ZeroStoreLoadStatus : std::uint8_t {
  kLoaded,
  kNotFound,
  kInvalid,
  kError
};

/// Hardware-independent seam for persistent user settings.
class ZeroReferenceStore {
 public:
  virtual ~ZeroReferenceStore() = default;

  /// Load one decoded record; platform adapters validate their serialized form.
  virtual ZeroStoreLoadStatus load(ZeroReferenceRecord& record) = 0;
  /// Persist one complete record and report whether the write succeeded.
  virtual bool save(const ZeroReferenceRecord& record) = 0;
};

/// Applies family-default fallback and saves only accepted, confirmed captures.
/// The caller controls when capture begins; this manager never auto-zeros.
class ZeroReferenceManager {
 public:
  /// Configure a fallback value and the measured/profile-based capture limits.
  ZeroReferenceManager(std::uint32_t familyDefaultMicrovolts,
                       ZeroCaptureCriteria criteria);

  /// Load a valid saved record or retain the family default on first use/errors.
  ZeroReferenceState initialize(ZeroReferenceStore& store);
  /// Mark the beginning of an explicitly confirmed capture request.
  void beginCapture();
  /// Cancel without changing the active or persisted reference.
  void cancelCapture();
  /// Evaluate a finite window and persist only if it is accepted.
  ZeroCaptureResult finishCapture(const std::uint32_t* samples,
                                 std::size_t sampleCount,
                                 ZeroReferenceStore& store);

  /// Current atmospheric reference used by the pressure converter.
  std::uint32_t activeZeroMicrovolts() const { return activeZeroMicrovolts_; }
  /// Current user-visible state for diagnostics and Settings rendering.
  ZeroReferenceState state() const { return state_; }
  /// Last stability/plausibility result, meaningful after a capture finishes.
  ZeroCaptureStatus lastCaptureStatus() const { return lastCaptureStatus_; }
  /// Count of conversions expected in one capture.
  std::size_t requiredSampleCount() const { return criteria_.minimumSampleCount; }

 private:
  /// A stored voltage must remain inside the family atmospheric-output range.
  bool isPlausibleStoredZero(std::uint32_t voltageMicrovolts) const;

  const std::uint32_t familyDefaultMicrovolts_;
  const ZeroCaptureCriteria criteria_;
  std::uint32_t activeZeroMicrovolts_;
  ZeroReferenceState state_ = ZeroReferenceState::kFamilyDefault;
  ZeroCaptureStatus lastCaptureStatus_ = ZeroCaptureStatus::kInvalidCriteria;
  bool hasSavedReference_ = false;
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
