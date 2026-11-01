#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace gauge_core {

/// Signed synthetic/display pressure stored in tenths of inH2O.
/// A value of -55 represents -5.5 inH2O. This representation does not imply
/// sensor resolution or measurement accuracy.
using PressureTenthsInH2O = std::int16_t;

/// Number of one-second sample slots retained for the scrolling trend display.
constexpr std::size_t kHistoryCapacity = 30;
/// Demo trend cadence; live acquisition may require a separately validated rate.
constexpr std::uint32_t kSampleIntervalMs = 1000;

/// Synthetic M01 UI status; these values are not production alarm states.
enum class PressureBand : std::uint8_t {
  kDanger,   ///< Outside the provisional demo warning interval.
  kWarning,  ///< Outside the provisional demo normal interval.
  kNormal    ///< Inside the provisional demo normal interval.
};
/// Gestures understood by the temporary M01 demo interaction.
enum class Gesture : std::uint8_t {
  kClick,        ///< Single button activation after library recognition.
  kDoubleClick,  ///< Two activations recognized as a double-click.
  kLongPress     ///< Button held for the library's configured duration.
};
/// The three views backed by one shared gauge session.
enum class Page : std::uint8_t {
  kGauge,  ///< Current value, needle, and synthetic status ribbon.
  kTrend,  ///< Recent one-second session samples.
  kMinMax  ///< Extrema accumulated since the current firmware run began.
};

/// Shared display span and synthetic-only lower/upper status boundaries.
struct DemoBandConfig {
  /// Lower endpoint of the inclusive synthetic display span, in tenths inH2O.
  PressureTenthsInH2O minimum = -140;  // -14.0 inH2O
  /// Upper endpoint of the inclusive synthetic display span, in tenths inH2O.
  PressureTenthsInH2O maximum = 45;  // +4.5 inH2O
  /// Crossing below this value enters lower danger; equality is warning.
  PressureTenthsInH2O lowerDanger = -120;  // -12.0 inH2O
  /// Crossing below this value enters lower warning; equality is normal.
  PressureTenthsInH2O lowerWarning = -80;  // -8.0 inH2O
  /// Crossing above this value enters upper warning; equality is normal.
  PressureTenthsInH2O upperWarning = -30;  // -3.0 inH2O
  /// Crossing above this value enters upper danger; equality is warning.
  PressureTenthsInH2O upperDanger = 0;  // 0.0 inH2O

  /// Check strict threshold ordering and that thresholds lie inside the span.
  /// @return True when this configuration can be classified and sampled.
  bool isValid() const;

  /// Check the inclusive visible range.
  /// @param pressure Signed pressure in tenths of inH2O.
  /// @return True when pressure is between minimum and maximum, inclusive.
  bool contains(PressureTenthsInH2O pressure) const;
};

/// Read-only value object supplied to UI code; history is oldest-to-newest.
struct DisplaySnapshot {
  /// Shared thresholds used to derive status and render colored bands.
  DemoBandConfig bands{};
  /// False until GaugeSession accepts its first in-range pressure.
  bool hasPressure = false;
  /// Most recently accepted signed synthetic value, in tenths inH2O.
  PressureTenthsInH2O pressure = 0;
  /// Status calculated by PressureClassifier for pressure above.
  PressureBand band = PressureBand::kNormal;
  /// Current page and whether at least one accepted value established extrema.
  Page page = Page::kGauge;
  bool hasExtrema = false;
  /// Lowest/highest accepted values since this GaugeSession was constructed.
  PressureTenthsInH2O minimumRecorded = 0;
  PressureTenthsInH2O maximumRecorded = 0;
  /// Chronological samples; only indexes below historyCount are populated.
  std::array<PressureTenthsInH2O, kHistoryCapacity> history{};
  std::size_t historyCount = 0;
};

}  // namespace gauge_core
