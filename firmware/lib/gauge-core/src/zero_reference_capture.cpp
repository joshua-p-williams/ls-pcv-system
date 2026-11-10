#include "gauge_core/zero_reference_capture.hpp"

#include <limits>

namespace gauge_core {

/// Ensure the caller supplied a usable, bounded capture configuration.
bool ZeroCaptureCriteria::isValid() const {
  return minimumVoltageMicrovolts < maximumVoltageMicrovolts &&
         minimumSampleCount >= 2 &&
         minimumSampleCount <= kMaximumZeroCaptureSamples;
}

/// Reject incomplete or unsuitable voltage windows before calculating zero.
ZeroCaptureResult ZeroReferenceCapture::evaluate(
    const std::uint32_t* samples, std::size_t sampleCount,
    const ZeroCaptureCriteria& criteria) const {
  ZeroCaptureResult result;
  if (!criteria.isValid()) {
    result.status = ZeroCaptureStatus::kInvalidCriteria;
    return result;
  }
  if (samples == nullptr || sampleCount < criteria.minimumSampleCount) {
    result.status = ZeroCaptureStatus::kInsufficientSamples;
    return result;
  }
  if (sampleCount > kMaximumZeroCaptureSamples) {
    result.status = ZeroCaptureStatus::kTooManySamples;
    return result;
  }

  // Track the spread as well as the sum. A plausible average alone is not
  // enough: alternating or drifting values could average to a believable zero
  // even though the sensor was not stable during capture.
  std::uint32_t minimum = std::numeric_limits<std::uint32_t>::max();
  std::uint32_t maximum = 0;
  std::uint64_t sum = 0;
  for (std::size_t index = 0; index < sampleCount; ++index) {
    const std::uint32_t voltage = samples[index];
    if (voltage < criteria.minimumVoltageMicrovolts ||
        voltage > criteria.maximumVoltageMicrovolts) {
      result.status = ZeroCaptureStatus::kVoltageOutOfRange;
      return result;
    }
    if (voltage < minimum) minimum = voltage;
    if (voltage > maximum) maximum = voltage;
    sum += voltage;
  }

  // Preserve the measured window limits even when rejected, so a caller can
  // explain whether it was out of profile or simply too noisy.
  result.minimumObservedMicrovolts = minimum;
  result.maximumObservedMicrovolts = maximum;
  if (maximum - minimum > criteria.maximumPeakToPeakMicrovolts) {
    result.status = ZeroCaptureStatus::kUnstable;
    return result;
  }

  // Round the mean to the nearest microvolt rather than truncating it. The
  // caller may then persist this candidate only after the user confirms the
  // capture procedure; persistence is deliberately outside this pure helper.
  result.zeroVoltageMicrovolts = static_cast<std::uint32_t>(
      (sum + sampleCount / 2u) / sampleCount);
  result.status = ZeroCaptureStatus::kAccepted;
  return result;
}

/// Start with the family default until a decoded and plausible record is found.
ZeroReferenceManager::ZeroReferenceManager(
    std::uint32_t familyDefaultMicrovolts, ZeroCaptureCriteria criteria)
    : familyDefaultMicrovolts_(familyDefaultMicrovolts), criteria_(criteria),
      activeZeroMicrovolts_(familyDefaultMicrovolts) {}

/// Load only a supported record whose value remains inside the capture profile.
ZeroReferenceState ZeroReferenceManager::initialize(ZeroReferenceStore& store) {
  activeZeroMicrovolts_ = familyDefaultMicrovolts_;
  hasSavedReference_ = false;
  ZeroReferenceRecord record;
  switch (store.load(record)) {
    case ZeroStoreLoadStatus::kNotFound:
      state_ = ZeroReferenceState::kFamilyDefault;
      return state_;
    case ZeroStoreLoadStatus::kInvalid:
      state_ = ZeroReferenceState::kInvalidStoredData;
      return state_;
    case ZeroStoreLoadStatus::kError:
      state_ = ZeroReferenceState::kStorageFailure;
      return state_;
    case ZeroStoreLoadStatus::kLoaded:
      break;
  }

  if (record.version != ZeroReferenceRecord::kCurrentVersion ||
      !isPlausibleStoredZero(record.zeroVoltageMicrovolts)) {
    state_ = ZeroReferenceState::kInvalidStoredData;
    return state_;
  }

  activeZeroMicrovolts_ = record.zeroVoltageMicrovolts;
  hasSavedReference_ = true;
  state_ = ZeroReferenceState::kSavedTrim;
  return state_;
}

/// Expose capture-in-progress only after the application has confirmed the prompt.
void ZeroReferenceManager::beginCapture() {
  lastCaptureStatus_ = ZeroCaptureStatus::kInvalidCriteria;
  state_ = ZeroReferenceState::kCaptureInProgress;
}

/// Leave the prior zero untouched when a user cancels before completion.
void ZeroReferenceManager::cancelCapture() {
  state_ = hasSavedReference_ ? ZeroReferenceState::kSavedTrim
                              : ZeroReferenceState::kFamilyDefault;
}

/// Evaluate and persist the candidate atomically from the settings manager's view.
ZeroCaptureResult ZeroReferenceManager::finishCapture(
    const std::uint32_t* samples, std::size_t sampleCount,
    ZeroReferenceStore& store) {
  ZeroCaptureResult result =
      ZeroReferenceCapture().evaluate(samples, sampleCount, criteria_);
  lastCaptureStatus_ = result.status;
  if (result.status != ZeroCaptureStatus::kAccepted ||
      !isPlausibleStoredZero(result.zeroVoltageMicrovolts)) {
    if (result.status == ZeroCaptureStatus::kAccepted) {
      lastCaptureStatus_ = ZeroCaptureStatus::kVoltageOutOfRange;
    }
    state_ = ZeroReferenceState::kRejected;
    return result;
  }

  const ZeroReferenceRecord record{
      ZeroReferenceRecord::kCurrentVersion, result.zeroVoltageMicrovolts};
  if (!store.save(record)) {
    state_ = ZeroReferenceState::kStorageFailure;
    return result;
  }

  // Do not change the active reference until persistence confirms success.
  activeZeroMicrovolts_ = record.zeroVoltageMicrovolts;
  hasSavedReference_ = true;
  state_ = ZeroReferenceState::kSavedSuccessfully;
  return result;
}

/// Keep loaded and newly captured values within the published family zero range.
bool ZeroReferenceManager::isPlausibleStoredZero(
    std::uint32_t voltageMicrovolts) const {
  return criteria_.isValid() &&
         voltageMicrovolts >= criteria_.minimumVoltageMicrovolts &&
         voltageMicrovolts <= criteria_.maximumVoltageMicrovolts;
}

}  // namespace gauge_core
