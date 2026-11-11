#include "gauge_core/measurement_freshness.hpp"

namespace gauge_core {

/// Apply the display freshness rule without mutating acquisition-owned state.
MeasurementSnapshot withFreshnessPolicy(const MeasurementSnapshot& measurement,
                                        std::uint32_t nowMs,
                                        std::uint32_t staleAfterMs) {
  MeasurementSnapshot presentation = measurement;
  // Unsigned subtraction handles the millis() counter wrapping through zero.
  const std::uint32_t ageMs = nowMs - measurement.acquiredAtMs;
  if (measurement.quality == MeasurementQuality::kValid &&
      ageMs > staleAfterMs) {
    presentation.quality = MeasurementQuality::kStale;
    presentation.hasPressure = false;
  }
  return presentation;
}

}  // namespace gauge_core
