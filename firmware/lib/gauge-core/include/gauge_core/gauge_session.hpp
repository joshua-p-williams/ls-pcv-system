#pragma once

#include "gauge_types.hpp"
#include "pressure_classifier.hpp"

namespace gauge_core {

/// Owns one current reading, derived band, current-run extrema, page, and history.
/// It deliberately does not know about Arduino clocks, GPIO, or LVGL widgets.
class GaugeSession {
 public:
  /// Create an empty session using the supplied shared demo configuration.
  /// @param config Demo span and status boundaries; defaults to M01 values.
  explicit GaugeSession(DemoBandConfig config = {});

  /// Accept a pressure value and update current status and current-run extrema.
  /// @param pressure Pressure in tenths of inH2O.
  /// @return False if pressure is outside the configured span or config is invalid.
  bool updatePressure(PressureTenthsInH2O pressure);

  /// Record the current pressure at each elapsed one-second sample boundary.
  /// The first call establishes the clock origin and does not record a sample.
  /// Unsigned elapsed-time arithmetic supports millis() rollover. After a long
  /// stall, at most the newest 30 repeated current-value slots are appended.
  /// @param nowMs Current monotonic millisecond counter.
  /// @return Number of samples added during this call.
  std::size_t sampleAt(std::uint32_t nowMs);

  /// Advance gauge -> trend -> min/max -> gauge without changing measurements.
  void advancePage();

  /// Copy current values, run extrema, and chronological partial/complete history.
  /// @return A value snapshot suitable for read-only rendering.
  DisplaySnapshot snapshot() const;

 private:
  /// Append one timed sample, overwriting the oldest when the ring is full.
  void appendSample(PressureTenthsInH2O pressure);

  DemoBandConfig config_;
  PressureClassifier classifier_;
  DisplaySnapshot state_{};
  /// Circular storage: ringStart_ names the oldest retained sample.
  std::array<PressureTenthsInH2O, kHistoryCapacity> ring_{};
  std::size_t ringStart_ = 0;
  std::size_t ringCount_ = 0;
  /// First sampleAt() call establishes a time origin but records nothing.
  bool sampleClockStarted_ = false;
  std::uint32_t lastSampleMs_ = 0;
};

}  // namespace gauge_core
