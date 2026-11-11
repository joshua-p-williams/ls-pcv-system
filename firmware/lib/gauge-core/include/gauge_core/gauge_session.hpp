#pragma once

#include "gauge_types.hpp"
#include "measurement_tracker.hpp"

namespace gauge_core {

/// Owns M01 page state and adapts its synthetic source to the shared tracker.
/// It does not know about Arduino clocks, GPIO, or LVGL widgets.
class GaugeSession {
 public:
  /// Create an empty synthetic session using the display and status configuration.
  /// @param config Synthetic display span and shared status boundaries.
  explicit GaugeSession(DemoBandConfig config = {});

  /// Accept a pressure value and update current status and current-run extrema.
  /// @param pressure Pressure in tenths of inH2O.
  /// @return False if pressure is outside the configured span or config is invalid.
  bool updatePressure(PressureTenthsInH2O pressure);

  /// Record synthetic values on elapsed one-second boundaries through the tracker.
  /// The held synthetic value can fill catch-up slots; live gaps remain explicit.
  /// @param nowMs Current monotonic millisecond counter.
  /// @return Number of samples added during this call.
  std::size_t sampleAt(std::uint32_t nowMs);

  /// Advance gauge -> trend -> min/max -> gauge without changing measurements.
  void advancePage();

  /// Copy shared measurement data and add demo page/range presentation state.
  /// @return A value snapshot suitable for read-only rendering.
  GaugeDisplaySnapshot snapshot() const;

 private:
  DemoBandConfig config_;
  MeasurementTracker tracker_;
  Page page_ = Page::kGauge;
};

}  // namespace gauge_core
