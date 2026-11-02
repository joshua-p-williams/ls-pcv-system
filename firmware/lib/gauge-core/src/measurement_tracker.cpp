#include "gauge_core/measurement_tracker.hpp"

namespace gauge_core {

/// Keep sensor validity separate from display status classification.
MeasurementTracker::MeasurementTracker(PressureBandThresholds thresholds)
    : thresholds_(thresholds) {}

/// Replace the live sample and update run extrema only for accepted pressures.
bool MeasurementTracker::record(const MeasurementObservation& observation) {
  if (observation.source == MeasurementSource::kUnknown) return false;

  // First validate the caller's claims. A failed sensor/ADC read must not carry
  // a pressure through this API, otherwise the UI could show stale-looking
  // numbers with a fresh timestamp or fold them into the run extrema.
  PressureBand band = state_.band;
  if (observation.quality == MeasurementQuality::kValid) {
    if (!observation.hasPressure ||
        !classifier_.classify(observation.pressure, thresholds_, band)) {
      return false;
    }
  } else if (observation.hasPressure) {
    // Invalid readings must never carry a pressure that a caller might display.
    return false;
  }

  state_.source = observation.source;
  state_.quality = observation.quality;
  state_.acquiredAtMs = observation.acquiredAtMs;
  state_.hasRawAdcCode = observation.hasRawAdcCode;
  state_.rawAdcCode = observation.rawAdcCode;
  state_.hasSignalVoltage = observation.hasSignalVoltage;
  state_.signalVoltageMicrovolts = observation.signalVoltageMicrovolts;
  state_.hasPressure = observation.quality == MeasurementQuality::kValid;

  // Keep extrema as the minimum/maximum accepted pressures from this run.
  // Invalid observations clear only the live value, not the useful history.
  if (!state_.hasPressure) return true;

  state_.pressure = observation.pressure;
  state_.band = band;
  if (!state_.hasExtrema) {
    state_.minimumRecorded = observation.pressure;
    state_.maximumRecorded = observation.pressure;
    state_.hasExtrema = true;
  } else {
    if (observation.pressure < state_.minimumRecorded) {
      state_.minimumRecorded = observation.pressure;
    }
    if (observation.pressure > state_.maximumRecorded) {
      state_.maximumRecorded = observation.pressure;
    }
  }
  return true;
}

/// Add elapsed one-second slots without inventing unobserved live readings.
std::size_t MeasurementTracker::sampleAt(std::uint32_t nowMs) {
  if (!sampleClockStarted_) {
    sampleClockStarted_ = true;
    lastSampleMs_ = nowMs;
    return 0;
  }

  const std::uint32_t previousSampleMs = lastSampleMs_;
  const std::uint32_t elapsed = nowMs - previousSampleMs;
  const std::uint32_t intervals = elapsed / kSampleIntervalMs;
  if (intervals == 0) return 0;

  // After a long pause, older slots would immediately be overwritten by the
  // fixed-size history ring. Append only the newest retained slots to bound
  // work and preserve the same final history efficiently.
  const std::size_t slotsToAppend = intervals > kHistoryCapacity
      ? kHistoryCapacity : static_cast<std::size_t>(intervals);
  const std::uint32_t firstRetainedSlot = intervals - slotsToAppend;
  // M01's synthetic demo intentionally treats a generated state as its input
  // source so the example graph remains populated during simulated catch-up.
  // Live measurements do not use this shortcut: only an acquisition that
  // actually occurred in an interval can fill its slot.
  const bool repeatSyntheticState =
      state_.source == MeasurementSource::kSynthetic &&
      state_.quality == MeasurementQuality::kValid && state_.hasPressure;
  const std::uint32_t readingAgeFromOrigin = state_.acquiredAtMs - previousSampleMs;
  const bool readingFallsInElapsedWindow =
      state_.quality == MeasurementQuality::kValid && state_.hasPressure &&
      readingAgeFromOrigin > 0 && readingAgeFromOrigin <= elapsed;
  std::uint32_t readingSlot = readingFallsInElapsedWindow
      ? (readingAgeFromOrigin - 1u) / kSampleIntervalMs
      : intervals;
  // The loop may run a few milliseconds after a slot boundary. Assign that
  // current observation to the latest due slot rather than losing it.
  if (readingFallsInElapsedWindow && readingSlot >= intervals) {
    readingSlot = intervals - 1u;
  }

  // A trend point timestamp names the end of its one-second interval. For a
  // live source, the just-acquired sample belongs to its actual interval only.
  for (std::size_t index = 0; index < slotsToAppend; ++index) {
    const std::uint32_t absoluteSlot =
        firstRetainedSlot + static_cast<std::uint32_t>(index);
    TrendPoint point;
    point.timestampMs = previousSampleMs +
        (absoluteSlot + 1u) * kSampleIntervalMs;
    if (repeatSyntheticState ||
        (readingFallsInElapsedWindow && absoluteSlot == readingSlot)) {
      point.valid = true;
      point.pressure = state_.pressure;
    }
    appendTrendPoint(point);
  }

  // Advance by whole slots to preserve cadence and avoid accumulating drift.
  lastSampleMs_ += intervals * kSampleIntervalMs;
  return slotsToAppend;
}

/// Store one trend point, replacing the oldest point when history is full.
/// A circular buffer avoids shifting 30 entries every second on a small MCU.
void MeasurementTracker::appendTrendPoint(const TrendPoint& point) {
  if (ringCount_ < kHistoryCapacity) {
    const std::size_t next = (ringStart_ + ringCount_) % kHistoryCapacity;
    trendRing_[next] = point;
    ++ringCount_;
  } else {
    trendRing_[ringStart_] = point;
    ringStart_ = (ringStart_ + 1) % kHistoryCapacity;
  }
}

/// Return a detached, chronological copy for the UI or diagnostics.
/// Consumers cannot mutate the tracker's internal ring through this snapshot.
MeasurementSnapshot MeasurementTracker::snapshot() const {
  MeasurementSnapshot result = state_;
  result.historyCount = ringCount_;
  for (std::size_t index = 0; index < ringCount_; ++index) {
    result.history[index] = trendRing_[(ringStart_ + index) % kHistoryCapacity];
  }
  return result;
}

}  // namespace gauge_core
