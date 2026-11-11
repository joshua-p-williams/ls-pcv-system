#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "gauge_types.hpp"
#include "pressure_classifier.hpp"

namespace gauge_core {

/// One result from the acquisition path, before history/display bookkeeping.
///
/// In the live system, an ADC driver will read a code over I2C, convert that
/// code to signal voltage, and the pressure converter will derive inH2O. Each
/// step can fail independently. Keeping the available raw values and an
/// explicit quality lets diagnostics explain where a reading stopped without
/// pretending that a failed read was a real pressure.
struct MeasurementObservation {
  MeasurementSource source = MeasurementSource::kUnknown;
  MeasurementQuality quality = MeasurementQuality::kUnavailable;
  std::uint32_t acquiredAtMs = 0;
  bool hasRawAdcCode = false;
  std::int16_t rawAdcCode = 0;
  bool hasSignalVoltage = false;
  std::uint32_t signalVoltageMicrovolts = 0;
  bool hasPressure = false;
  PressureTenthsInH2O pressure = 0;
};

/// Maintains the latest measurement and the history used by gauge pages.
///
/// This layer answers three different questions: is the latest acquisition
/// usable, what pressure band does a usable value fall into, and what values
/// have been seen during this run? Separating these prevents an I2C error or a
/// saturated sensor from being mistaken for a numeric warning/danger reading.
/// Its one-second trend also marks missing measurements as gaps; repeating an
/// old value would falsely suggest the sensor kept measuring while it did not.
class MeasurementTracker {
 public:
  /// Create a tracker using display/status thresholds, not sensor limits.
  /// Sensor electrical limits belong to PressureCalibrationProfile. Status
  /// thresholds describe interpretation of valid pressure values; they do not
  /// make an invalid or saturated sensor value valid.
  /// @param thresholds Lower/upper warning and danger boundaries.
  explicit MeasurementTracker(PressureBandThresholds thresholds = {});

  /// Publish one acquisition result.
  ///
  /// A valid result must include pressure and pass classification. A fault
  /// result updates the current quality and clears the current pressure, but
  /// leaves accepted extrema and prior trend history intact for diagnosis.
  /// @param observation Latest acquisition result and its optional values.
  /// @return False for malformed valid input or invalid classifier thresholds.
  bool record(const MeasurementObservation& observation);

  /// Advance the trend clock by one-second slots.
  ///
  /// The sensor read loop may run faster than the trend display. Sampling at
  /// fixed intervals gives each point a consistent time meaning. If no fresh
  /// valid observation falls in an interval, that slot remains a gap instead
  /// of copying a stale value. Unsigned elapsed-time arithmetic remains valid
  /// when the microsecond/millisecond counter wraps around.
  /// @param nowMs Current monotonic millisecond counter.
  /// @return Number of slots appended, capped at the history capacity.
  std::size_t sampleAt(std::uint32_t nowMs);

  /// Start a new history/extrema run after a stored zero reference changes.
  /// Old pressures used a different offset and must not be mixed with new ones.
  void reset();

  /// Copy current state and linearize retained trend slots oldest-to-newest.
  /// @return Value snapshot suitable for read-only consumers.
  MeasurementSnapshot snapshot() const;

 private:
  /// Append one point and overwrite the oldest retained point when full.
  void appendTrendPoint(const TrendPoint& point);

  PressureBandThresholds thresholds_;
  PressureClassifier classifier_;
  MeasurementSnapshot state_{};
  std::array<TrendPoint, kHistoryCapacity> trendRing_{};
  std::size_t ringStart_ = 0;
  std::size_t ringCount_ = 0;
  bool sampleClockStarted_ = false;
  std::uint32_t lastSampleMs_ = 0;
};

}  // namespace gauge_core
