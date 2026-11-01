#include "gauge_core/demo_pressure_source.hpp"

namespace gauge_core {

/// Initialize a repeatable pseudo-random stream; zero is replaced because the
/// xorshift recurrence would otherwise remain stuck at zero forever.
DemoPressureSource::DemoPressureSource(std::uint32_t seed)
    : state_(seed == 0 ? 0x6D2B79F5u : seed) {}

/// Advance the xorshift32 pseudo-random state for a repeatable UI demonstration.
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

/// Choose a value in the inclusive interval [low, high]. Modulo reduction is
/// intentionally simple because this source only animates a synthetic UI demo.
bool DemoPressureSource::nextInclusive(PressureTenthsInH2O low,
                                       PressureTenthsInH2O high,
                                       PressureTenthsInH2O& result) {
  if (low > high) return false;
  const std::uint32_t width = static_cast<std::uint32_t>(high - low) + 1u;
  result = static_cast<PressureTenthsInH2O>(low + (nextRandom() % width));
  return true;
}

/// Generate a value inside the normal demo interval, including its boundaries.
bool DemoPressureSource::nextNormal(const DemoBandConfig& config,
                                    PressureTenthsInH2O& result) {
  if (!config.isValid()) return false;
  return nextInclusive(config.lowerWarning, config.upperWarning, result);
}

/// Choose one of four warning/danger regions, then generate within that region.
/// The regions are equally likely, not weighted by width; this is not a model of
/// how engine pressure or real faults are distributed.
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
