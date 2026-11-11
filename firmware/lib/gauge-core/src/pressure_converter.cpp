#include "gauge_core/pressure_converter.hpp"

#include <limits>

namespace gauge_core {
namespace {

/// Keep voltage units explicit: ADC-facing APIs use microvolts, while profile
/// slopes are expressed per volt. This conversion factor bridges those units.
constexpr std::uint64_t kMicrovoltsPerVolt = 1000000u;

/// Round a nonnegative rational value to the nearest integer, with halves up.
std::uint64_t roundedRatio(std::uint64_t numerator, std::uint64_t denominator) {
  return (numerator + denominator / 2u) / denominator;
}

}  // namespace

/// Check that zero is inside an inclusive sensor domain and both slopes exist.
/// A zero outside the valid domain would make one side of the signed pressure
/// curve undefined; a zero slope denominator would make division impossible.
bool PressureCalibrationProfile::isValid() const {
  return minimumSignalMicrovolts < maximumSignalMicrovolts &&
         zeroSignalMicrovolts >= minimumSignalMicrovolts &&
         zeroSignalMicrovolts <= maximumSignalMicrovolts &&
         positiveTenthsNumerator > 0 && positiveTenthsDenominator > 0 &&
         vacuumTenthsNumerator > 0 && vacuumTenthsDenominator > 0;
}

/// Turn an ADC-derived signal voltage into signed pressure.
PressureConversionResult PressureConverter::convert(
    std::uint32_t signalMicrovolts,
    const PressureCalibrationProfile& profile) const {
  PressureConversionResult result;
  if (!profile.isValid()) {
    result.status = PressureConversionStatus::kInvalidProfile;
    return result;
  }
  if (signalMicrovolts < profile.minimumSignalMicrovolts ||
      signalMicrovolts > profile.maximumSignalMicrovolts) {
    result.status = PressureConversionStatus::kOutOfDomain;
    return result;
  }

  // The sensor-family transfer curve is asymmetric around atmospheric zero.
  // Its output falls as positive pressure rises and increases as vacuum rises.
  // That is why separate positive-pressure and vacuum slopes are required;
  // treating the sensor as one symmetric line would distort one side.
  const bool positivePressure =
      signalMicrovolts <= profile.zeroSignalMicrovolts;
  const std::uint64_t deltaMicrovolts = positivePressure
      ? profile.zeroSignalMicrovolts - signalMicrovolts
      : signalMicrovolts - profile.zeroSignalMicrovolts;
  const std::uint64_t slopeNumerator = positivePressure
      ? profile.positiveTenthsNumerator : profile.vacuumTenthsNumerator;
  const std::uint64_t slopeDenominator = positivePressure
      ? profile.positiveTenthsDenominator : profile.vacuumTenthsDenominator;
  // Multiplying by the slope ratio gives tenths of inH2O. Using integer
  // arithmetic keeps host tests and the microcontroller on the same result.
  const std::uint64_t pressureTenths = roundedRatio(
      deltaMicrovolts * slopeNumerator,
      kMicrovoltsPerVolt * slopeDenominator);

  // A signed integer has one extra negative value: -32768 is representable
  // even though +32768 is not. Check each side against its own limit before
  // converting so the exact negative boundary remains valid.
  const std::uint64_t maximumMagnitude = positivePressure
      ? static_cast<std::uint64_t>(
            std::numeric_limits<PressureTenthsInH2O>::max())
      : static_cast<std::uint64_t>(-
            static_cast<std::int32_t>(
                std::numeric_limits<PressureTenthsInH2O>::min()));
  if (pressureTenths > maximumMagnitude) {
    result.status = PressureConversionStatus::kPressureOverflow;
    return result;
  }

  // Voltage below atmospheric reference means positive crankcase pressure;
  // voltage above it means vacuum, represented with a negative sign.
  const std::int32_t signedTenths = positivePressure
      ? static_cast<std::int32_t>(pressureTenths)
      : -static_cast<std::int32_t>(pressureTenths);
  if (signedTenths < std::numeric_limits<PressureTenthsInH2O>::min()) {
    result.status = PressureConversionStatus::kPressureOverflow;
    return result;
  }

  result.pressure = static_cast<PressureTenthsInH2O>(signedTenths);
  result.status = PressureConversionStatus::kValid;
  return result;
}

}  // namespace gauge_core
