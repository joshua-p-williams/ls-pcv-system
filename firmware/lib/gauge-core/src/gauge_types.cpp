#include "gauge_core/gauge_types.hpp"

namespace gauge_core {

bool DemoBandConfig::isValid() const {
  return minimum < lowerDanger && lowerDanger < lowerWarning &&
         lowerWarning <= upperWarning && upperWarning < upperDanger &&
         upperDanger < maximum;
}

bool DemoBandConfig::contains(PressureTenthsInH2O pressure) const {
  return pressure >= minimum && pressure <= maximum;
}

}  // namespace gauge_core
