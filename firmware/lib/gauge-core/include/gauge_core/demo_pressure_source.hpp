#pragma once

#include "gauge_types.hpp"

namespace gauge_core {

class DemoPressureSource {
 public:
  /// Create a deterministic pseudo-random source.
  /// @param seed Initial generator state; zero selects the documented default seed.
  explicit DemoPressureSource(std::uint32_t seed = 0x6D2B79F5u);

  /// Produce a synthetic value within the configured normal band.
  /// @param config Shared demo thresholds and display span.
  /// @param result Receives the generated pressure in tenths of inH2O.
  /// @return False when the configuration is invalid.
  bool nextNormal(const DemoBandConfig& config, PressureTenthsInH2O& result);

  /// Select one of four lower/upper warning/danger regions and sample within it.
  /// @param config Shared demo thresholds and display span.
  /// @param result Receives the generated pressure in tenths of inH2O.
  /// @return False when the configuration is invalid.
  bool nextWarningOrDanger(const DemoBandConfig& config, PressureTenthsInH2O& result);

 private:
  std::uint32_t nextRandom();
  bool nextInclusive(PressureTenthsInH2O low, PressureTenthsInH2O high,
                     PressureTenthsInH2O& result);

  std::uint32_t state_;
};

}  // namespace gauge_core
