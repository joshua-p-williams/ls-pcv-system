#include "gauge_core/gauge_types.hpp"

namespace gauge_core {

/// Reject ambiguous/overlapping bands before classification or value generation.
/// The normal interval may collapse to one value when its endpoints are equal.
bool DemoBandConfig::isValid() const {
  return minimum < lowerDanger && lowerDanger < lowerWarning &&
         lowerWarning <= upperWarning && upperWarning < upperDanger &&
         upperDanger < maximum;
}

/// Test the inclusive visual domain separately from status classification.
/// Values inside the span may still belong to warning or danger bands.
bool DemoBandConfig::contains(PressureTenthsInH2O pressure) const {
  return pressure >= minimum && pressure <= maximum;
}

}  // namespace gauge_core
