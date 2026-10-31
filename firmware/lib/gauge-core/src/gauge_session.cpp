#include "gauge_core/gauge_session.hpp"

namespace gauge_core {

GaugeSession::GaugeSession(DemoBandConfig config) : config_(config) {
  state_.bands = config_;
}

bool GaugeSession::updatePressure(PressureTenthsInH2O pressure) {
  PressureBand band;
  if (!classifier_.classify(pressure, config_, band)) return false;

  state_.pressure = pressure;
  state_.band = band;
  state_.hasPressure = true;
  if (!state_.hasExtrema) {
    state_.minimumRecorded = pressure;
    state_.maximumRecorded = pressure;
    state_.hasExtrema = true;
  } else {
    if (pressure < state_.minimumRecorded) state_.minimumRecorded = pressure;
    if (pressure > state_.maximumRecorded) state_.maximumRecorded = pressure;
  }
  return true;
}

void GaugeSession::appendSample(PressureTenthsInH2O pressure) {
  if (ringCount_ < kHistoryCapacity) {
    const std::size_t index = (ringStart_ + ringCount_) % kHistoryCapacity;
    ring_[index] = pressure;
    ++ringCount_;
  } else {
    ring_[ringStart_] = pressure;
    ringStart_ = (ringStart_ + 1) % kHistoryCapacity;
  }
}

std::size_t GaugeSession::sampleAt(std::uint32_t nowMs) {
  if (!state_.hasPressure) return 0;
  if (!sampleClockStarted_) {
    sampleClockStarted_ = true;
    lastSampleMs_ = nowMs;
    return 0;
  }

  // Unsigned subtraction intentionally handles the millis() counter wrapping.
  const std::uint32_t elapsed = nowMs - lastSampleMs_;
  const std::uint32_t intervals = elapsed / kSampleIntervalMs;
  if (intervals == 0) return 0;

  // Preserve the one-second schedule. Only the newest 30 catch-up entries can
  // affect the visible ring, so avoid an unbounded loop after an extended stall.
  lastSampleMs_ += intervals * kSampleIntervalMs;
  const std::size_t samplesToAppend =
      intervals > kHistoryCapacity ? kHistoryCapacity : intervals;
  for (std::size_t index = 0; index < samplesToAppend; ++index) {
    appendSample(state_.pressure);
  }
  return samplesToAppend;
}

void GaugeSession::advancePage() {
  switch (state_.page) {
    case Page::kGauge: state_.page = Page::kTrend; break;
    case Page::kTrend: state_.page = Page::kMinMax; break;
    case Page::kMinMax: state_.page = Page::kGauge; break;
  }
}

DisplaySnapshot GaugeSession::snapshot() const {
  DisplaySnapshot result = state_;
  result.historyCount = ringCount_;
  for (std::size_t index = 0; index < ringCount_; ++index) {
    result.history[index] = ring_[(ringStart_ + index) % kHistoryCapacity];
  }
  return result;
}

}  // namespace gauge_core
