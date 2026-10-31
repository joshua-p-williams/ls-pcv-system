#include "gauge_core/demo_pressure_source.hpp"

namespace gauge_core {

DemoPressureSource::DemoPressureSource(std::uint32_t seed)
    : state_(seed == 0 ? 0x6D2B79F5u : seed) {}

std::uint32_t DemoPressureSource::nextRandom() {
  // xorshift32 is a small deterministic pseudo-random generator suitable for a
  // repeatable UI demonstration. It is not intended for security or measurement.
  std::uint32_t value = state_;
  value ^= value << 13;
  value ^= value >> 17;
  value ^= value << 5;
  state_ = value;
  return value;
}

bool DemoPressureSource::nextInclusive(PressureTenthsInH2O low,
                                       PressureTenthsInH2O high,
                                       PressureTenthsInH2O& result) {
  if (low > high) return false;
  const std::uint32_t width = static_cast<std::uint32_t>(high - low) + 1u;
  result = static_cast<PressureTenthsInH2O>(low + (nextRandom() % width));
  return true;
}

bool DemoPressureSource::nextNormal(const DemoBandConfig& config,
                                    PressureTenthsInH2O& result) {
  if (!config.isValid()) return false;
  return nextInclusive(config.lowerWarning, config.upperWarning, result);
}

bool DemoPressureSource::nextWarningOrDanger(const DemoBandConfig& config,
                                             PressureTenthsInH2O& result) {
  if (!config.isValid()) return false;
  const std::uint8_t region = static_cast<std::uint8_t>(nextRandom() % 4u);
  switch (region) {
    case 0: return nextInclusive(config.minimum, config.lowerDanger - 1, result);
    case 1: return nextInclusive(config.lowerDanger, config.lowerWarning - 1, result);
    case 2: return nextInclusive(config.upperWarning + 1, config.upperDanger, result);
    default: return nextInclusive(config.upperDanger + 1, config.maximum, result);
  }
}

}  // namespace gauge_core
