#include "gauge_core/pressure_classifier.hpp"

namespace gauge_core {

/// Apply shared interval rules and leave result untouched on invalid input.
bool PressureClassifier::classify(PressureTenthsInH2O pressure,
                                  const PressureBandThresholds& thresholds,
                                  PressureBand& result) const {
  if (!thresholds.isValid()) return false;

  // Lower danger is [minimum, lowerDanger); lower warning is
  // [lowerDanger, lowerWarning); normal includes both endpoints; upper warning
  // is (upperWarning, upperDanger]; upper danger is (upperDanger, maximum].
  if (pressure < thresholds.lowerDanger || pressure > thresholds.upperDanger) {
    result = PressureBand::kDanger;
  } else if (pressure < thresholds.lowerWarning ||
             pressure > thresholds.upperWarning) {
    result = PressureBand::kWarning;
  } else {
    result = PressureBand::kNormal;
  }
  return true;
}

}  // namespace gauge_core
