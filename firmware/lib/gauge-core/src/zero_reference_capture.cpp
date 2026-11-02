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

}  // namespace gauge_core
