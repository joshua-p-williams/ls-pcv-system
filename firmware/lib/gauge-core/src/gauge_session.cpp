#include "gauge_core/gauge_session.hpp"

namespace gauge_core {

/// Build the M01 session around the shared tracker and retain demo display limits.
GaugeSession::GaugeSession(DemoBandConfig config)
    : config_(config), tracker_(config) {}

/// Accept only classifiable pressure; update current value, status, and extrema
/// together so a rejected value cannot partially alter the session snapshot.
bool GaugeSession::updatePressure(PressureTenthsInH2O pressure) {
  if (!config_.isValid() || !config_.contains(pressure)) return false;

  MeasurementObservation observation;
  observation.source = MeasurementSource::kSynthetic;
  observation.quality = MeasurementQuality::kValid;
  observation.hasPressure = true;
  observation.pressure = pressure;
  return tracker_.record(observation);
}

/// Refresh the synthetic reading time and let the tracker append elapsed slots.
/// M01 may repeat its held demo value; live readings are freshness-checked.
std::size_t GaugeSession::sampleAt(std::uint32_t nowMs) {
  const MeasurementSnapshot current = tracker_.snapshot();
  if (current.hasPressure && current.quality == MeasurementQuality::kValid) {
    // M01 deliberately treats its synthetic state as available each second;
    // the live source will publish each completed ADC conversion separately.
    MeasurementObservation observation;
    observation.source = MeasurementSource::kSynthetic;
    observation.quality = MeasurementQuality::kValid;
    observation.acquiredAtMs = nowMs;
    observation.hasPressure = true;
    observation.pressure = current.pressure;
    (void)tracker_.record(observation);
  }
  return tracker_.sampleAt(nowMs);
}

/// Cycle the visible page without changing pressure, history, or extrema.
void GaugeSession::advancePage() {
  switch (page_) {
    case Page::kGauge: page_ = Page::kTrend; break;
    case Page::kTrend: page_ = Page::kMinMax; break;
    case Page::kMinMax: page_ = Page::kGauge; break;
  }
}

/// Copy shared measurement state and append the M01 page/range view data.
GaugeDisplaySnapshot GaugeSession::snapshot() const {
  GaugeDisplaySnapshot result;
  static_cast<MeasurementSnapshot&>(result) = tracker_.snapshot();
  result.bands = config_;
  result.page = page_;
  return result;
}

}  // namespace gauge_core
