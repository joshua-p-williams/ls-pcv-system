#pragma once

#include <cstdint>

#include "gauge_types.hpp"

namespace gauge_core {

/// Describes the sensor's electrical-to-pressure transfer model.
///
/// The ADS1115 reports a voltage code; it does not know that the voltage came
/// from a pressure sensor and cannot return inH2O by itself. This profile tells
/// the application how to interpret that voltage. The default model represents
/// the advertised GM-family sensor behavior documented by M02 research. It is
/// a starting assumption until this specific sensor chain is checked against
/// known pressure, so keep it separate from the conversion algorithm.
struct PressureCalibrationProfile {
  /// Inclusive sensor signal limits in microvolts, not the ADC supply rails.
  /// The converter refuses values outside this region instead of extending
  /// the sensor curve into a range where the sensor may be saturated.
  std::uint32_t minimumSignalMicrovolts = 500000;
  std::uint32_t maximumSignalMicrovolts = 4500000;
  /// Output voltage corresponding to atmospheric pressure in microvolts.
  /// Pressure is relative to atmosphere: lower voltage is positive pressure
  /// and higher voltage is vacuum for this sensor family. A future user zero
  /// adjustment changes this reference, not the sensor's valid signal limits.
  std::uint32_t zeroSignalMicrovolts = 1500000;
  /// Positive branch slope in tenths of inH2O per volt, as an exact ratio.
  std::uint16_t positiveTenthsNumerator = 45;
  std::uint16_t positiveTenthsDenominator = 1;
  /// Vacuum branch magnitude in tenths of inH2O per volt, as an exact ratio.
  std::uint16_t vacuumTenthsNumerator = 140;
  std::uint16_t vacuumTenthsDenominator = 3;

  /// Validate domain, zero point, and nonzero slope ratios.
  /// @return True when conversion is bounded and all ratios are defined.
  bool isValid() const;
};

/// Why a voltage did or did not produce a pressure value.
///
/// This is deliberately distinct from PressureBand: a warning/danger reading
/// is still a valid measurement, while an out-of-domain voltage has no safe
/// pressure value to classify or display as a number.
enum class PressureConversionStatus : std::uint8_t {
  kValid,
  kInvalidProfile,
  kOutOfDomain,
  kPressureOverflow
};

/// Output of one conversion; pressure is meaningful only for kValid.
struct PressureConversionResult {
  PressureConversionStatus status = PressureConversionStatus::kInvalidProfile;
  PressureTenthsInH2O pressure = 0;
};

/// Convert sensor voltage into signed pressure relative to atmosphere.
///
/// This hardware-independent class receives voltage in microvolts from a
/// future ADC adapter. Keeping it independent of I2C and the ADS1115 lets the
/// math be tested on a computer before connecting physical electronics.
class PressureConverter {
 public:
  /// Convert with two slopes, one on each side of atmospheric pressure.
  ///
  /// Integer ratios avoid floating-point variation and preserve the sensor's
  /// asymmetric response. The result is rounded to tenths of inH2O, matching
  /// the project's stored pressure unit. Inputs outside the characterized
  /// sensor range are reported as invalid rather than extrapolated.
  /// @param signalMicrovolts Nonnegative signal voltage from the ADC adapter.
  /// @param profile Sensor-chain slopes, atmospheric reference, and valid domain.
  /// @return Explicit status and pressure only when the input is valid.
  PressureConversionResult convert(
      std::uint32_t signalMicrovolts,
      const PressureCalibrationProfile& profile) const;
};

}  // namespace gauge_core
