#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace gauge_core {

/// Signed synthetic/display pressure stored in tenths of inH2O.
/// A value of -55 represents -5.5 inH2O. This representation does not imply
/// sensor resolution or measurement accuracy.
using PressureTenthsInH2O = std::int16_t;

constexpr std::size_t kHistoryCapacity = 30;
constexpr std::uint32_t kSampleIntervalMs = 1000;

/// Synthetic M01 UI status; these values are not production alarm states.
enum class PressureBand : std::uint8_t { kDanger, kWarning, kNormal };
/// Gestures understood by the temporary M01 demo interaction.
enum class Gesture : std::uint8_t { kClick, kDoubleClick, kLongPress };
/// The three views backed by one shared gauge session.
enum class Page : std::uint8_t { kGauge, kTrend, kMinMax };

/// Shared display span and synthetic-only lower/upper status boundaries.
struct DemoBandConfig {
  PressureTenthsInH2O minimum = -140;       // -14.0 inH2O
  PressureTenthsInH2O maximum = 45;         // +4.5 inH2O
  PressureTenthsInH2O lowerDanger = -120;   // -12.0 inH2O
  PressureTenthsInH2O lowerWarning = -80;   // -8.0 inH2O
  PressureTenthsInH2O upperWarning = -30;   // -3.0 inH2O
  PressureTenthsInH2O upperDanger = 0;      // 0.0 inH2O

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
  DemoBandConfig bands{};
  bool hasPressure = false;
  PressureTenthsInH2O pressure = 0;
  PressureBand band = PressureBand::kNormal;
  Page page = Page::kGauge;
  bool hasExtrema = false;
  PressureTenthsInH2O minimumRecorded = 0;
  PressureTenthsInH2O maximumRecorded = 0;
  std::array<PressureTenthsInH2O, kHistoryCapacity> history{};
  std::size_t historyCount = 0;
};

}  // namespace gauge_core
