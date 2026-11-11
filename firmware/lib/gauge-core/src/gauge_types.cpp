#include "gauge_core/gauge_types.hpp"

namespace gauge_core {

/// Reject inverted or overlapping status boundaries before classification.
bool PressureBandThresholds::isValid() const {
  return lowerDanger < lowerWarning && lowerWarning <= upperWarning &&
         upperWarning < upperDanger;
}

/// Reject ambiguous bands and require the synthetic display span to surround them.
/// The normal interval may collapse to one value when its endpoints are equal.
bool DemoBandConfig::isValid() const {
  return PressureBandThresholds::isValid() && minimum < lowerDanger &&
         upperDanger < maximum;
}

/// Test the inclusive visual domain separately from status classification.
/// Values inside the span may still belong to warning or danger bands.
bool DemoBandConfig::contains(PressureTenthsInH2O pressure) const {
  return pressure >= minimum && pressure <= maximum;
}

}  // namespace gauge_core
