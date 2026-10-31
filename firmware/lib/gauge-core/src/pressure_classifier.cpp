#include "gauge_core/pressure_classifier.hpp"

namespace gauge_core {

bool PressureClassifier::classify(PressureTenthsInH2O pressure,
                                  const DemoBandConfig& config,
                                  PressureBand& result) const {
  if (!config.isValid() || !config.contains(pressure)) return false;

  // Lower danger is [minimum, lowerDanger); lower warning is
  // [lowerDanger, lowerWarning); normal includes both endpoints; upper warning
  // is (upperWarning, upperDanger]; upper danger is (upperDanger, maximum].
  if (pressure < config.lowerDanger || pressure > config.upperDanger) {
    result = PressureBand::kDanger;
  } else if (pressure < config.lowerWarning || pressure > config.upperWarning) {
    result = PressureBand::kWarning;
  } else {
    result = PressureBand::kNormal;
  }
  return true;
}

}  // namespace gauge_core
