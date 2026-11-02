#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace gauge_core {

/// Signed crankcase pressure represented in tenths of inH2O.
///
/// Negative pressure means crankcase vacuum relative to atmosphere; positive
/// pressure means the crankcase is above atmospheric pressure. A value of -55
/// represents -5.5 inH2O. The integer unit gives the software a convenient
/// tenth-unit display step; it does not mean the sensor can resolve or measure
/// pressure accurately to that precision.
using PressureTenthsInH2O = std::int16_t;

/// Number of one-second sample slots retained for the scrolling trend display.
constexpr std::size_t kHistoryCapacity = 30;
/// User-visible trend cadence, independent of the ADC conversion rate.
constexpr std::uint32_t kSampleIntervalMs = 1000;

/// Identifies whether a reading came from hardware or an intentionally synthetic demo.
enum class MeasurementSource : std::uint8_t {
  kUnknown,    ///< No source has published a reading yet.
  kLive,       ///< Reading supplied by the physical sensor/ADC chain.
  kSynthetic   ///< Test/demo value; never a physical measurement.
};

/// Describes acquisition validity, separately from pressure severity.
///
/// For example, kOutOfDomain means the sensor voltage is beyond the active
/// conversion model, so no numeric pressure should be shown. A valid pressure
/// can still fall in a warning or danger band; those are different concepts.
enum class MeasurementQuality : std::uint8_t {
  kUnavailable,  ///< No usable reading has been received yet.
  kValid,        ///< A fresh reading passed the configured measurement checks.
  kStale,        ///< A once-valid reading is no longer fresh.
  kAdcError,     ///< The ADC returned an error or an unusable conversion.
  kI2cError,     ///< The ADC could not be reached through the I2C bus.
  kOutOfDomain   ///< Signal voltage is outside the active conversion profile.
};

/// Describes how the atmospheric pressure reference was obtained.
enum class ZeroReferenceState : std::uint8_t {
  kFamilyDefault,       ///< Uses the provisional published-family reference.
  kSavedTrim,           ///< Uses a previously saved user-confirmed offset.
  kCaptureInProgress,   ///< A user-confirmed atmospheric capture is active.
  kSavedSuccessfully,   ///< A new reference was accepted and stored.
  kRejected,            ///< Capture was rejected by its configured criteria.
  kStorageFailure       ///< A valid capture could not be persisted.
};

/// Severity of a valid pressure relative to configured pressure thresholds.
/// These bands do not validate sensor readings and current thresholds remain
/// provisional guidance until the project adopts better supported values.
enum class PressureBand : std::uint8_t {
  kDanger,   ///< Beyond an outer configured warning boundary.
  kWarning,  ///< Outside the configured normal interval, short of danger.
  kNormal    ///< Inside the configured normal interval.
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

/// Pressure severity boundaries, independent of sensor electrical limits.
/// A voltage can be inside the sensor's measurable range and still produce a
/// danger pressure; conversely, an impossible voltage is invalid rather than
/// a numeric danger reading.
struct PressureBandThresholds {
  /// Crossing below this value enters lower danger; equality is warning.
  PressureTenthsInH2O lowerDanger = -120;  // -12.0 inH2O
  /// Crossing below this value enters lower warning; equality is normal.
  PressureTenthsInH2O lowerWarning = -80;  // -8.0 inH2O
  /// Crossing above this value enters upper warning; equality is normal.
  PressureTenthsInH2O upperWarning = -30;  // -3.0 inH2O
  /// Crossing above this value enters upper danger; equality is warning.
  PressureTenthsInH2O upperDanger = 0;  // 0.0 inH2O

  /// Check strict ordering of lower and upper warning/danger boundaries.
  /// @return True when the pressure intervals are unambiguous.
  bool isValid() const;
};

/// Demo display span and generation bounds, plus shared status boundaries.
/// The endpoints are synthetic display limits, not the live sensor domain.
struct DemoBandConfig : PressureBandThresholds {
  /// Lower endpoint of the inclusive synthetic display span, in tenths inH2O.
  PressureTenthsInH2O minimum = -140;  // -14.0 inH2O
  /// Upper endpoint of the inclusive synthetic display span, in tenths inH2O.
  PressureTenthsInH2O maximum = 45;  // +4.5 inH2O
  /// Check threshold ordering and the synthetic range around those thresholds.
  /// @return True when this configuration can be classified and sampled.
  bool isValid() const;

  /// Check the inclusive visible range.
  /// @param pressure Signed pressure in tenths of inH2O.
  /// @return True when pressure is between minimum and maximum, inclusive.
  bool contains(PressureTenthsInH2O pressure) const;
};

/// One trend slot; invalid slots explicitly represent missing/stale data.
struct TrendPoint {
  /// True only when a fresh, accepted pressure was available for this slot.
  bool valid = false;
  /// Pressure in tenths of inH2O; meaningful only when valid is true.
  PressureTenthsInH2O pressure = 0;
  /// Monotonic millisecond time for the end of this one-second slot.
  std::uint32_t timestampMs = 0;
};

/// Measurement-domain data shared by live acquisition and synthetic sources.
/// Consumers treat snapshots as read-only value objects. A pressure and band
/// are meaningful only when quality is kValid and hasPressure is true.
struct MeasurementSnapshot {
  /// Origin marker keeps demo values distinguishable from sensor readings.
  MeasurementSource source = MeasurementSource::kUnknown;
  /// Validity/fault state; it is independent of the normal/warning/danger band.
  MeasurementQuality quality = MeasurementQuality::kUnavailable;
  /// Most recent successful acquisition time on the monotonic millisecond clock.
  std::uint32_t acquiredAtMs = 0;
  /// Whether raw ADC code and signal voltage fields contain current data.
  bool hasRawAdcCode = false;
  std::int16_t rawAdcCode = 0;
  bool hasSignalVoltage = false;
  std::uint32_t signalVoltageMicrovolts = 0;
  /// False unless a valid current pressure has passed conversion and classification.
  bool hasPressure = false;
  /// Most recently accepted pressure in tenths of inH2O.
  PressureTenthsInH2O pressure = 0;
  /// Status calculated only for an accepted pressure.
  PressureBand band = PressureBand::kNormal;
  /// Whether at least one valid pressure has established run extrema.
  bool hasExtrema = false;
  /// Lowest/highest accepted values since this tracker was initialized.
  PressureTenthsInH2O minimumRecorded = 0;
  PressureTenthsInH2O maximumRecorded = 0;
  /// Chronological trend slots; only indexes below historyCount are populated.
  std::array<TrendPoint, kHistoryCapacity> history{};
  std::size_t historyCount = 0;
  /// Identifies family fallback, saved offset, or the current capture outcome.
  ZeroReferenceState zeroReference = ZeroReferenceState::kFamilyDefault;
};

/// Measurement snapshot plus page and visual range needed by the current UI.
/// Inheritance keeps display-facing access concise while allowing the live
/// measurement data contract to be published independently of page state.
struct GaugeDisplaySnapshot : MeasurementSnapshot {
  /// Display range/status bands; live pressure validity comes from measurement quality.
  DemoBandConfig bands{};
  /// Active M01/M02 screen, owned by the application/session layer.
  Page page = Page::kGauge;
};

}  // namespace gauge_core
