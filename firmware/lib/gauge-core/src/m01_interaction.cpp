#include "gauge_core/m01_interaction.hpp"

namespace gauge_core {

M01Interaction::M01Interaction(DemoPressureSource& source, GaugeSession& session)
    : source_(source), session_(session) {}

bool M01Interaction::handle(Gesture gesture) {
  const DemoBandConfig config = session_.snapshot().bands;
  PressureTenthsInH2O pressure;
  switch (gesture) {
    case Gesture::kClick:
      return source_.nextNormal(config, pressure) && session_.updatePressure(pressure);
    case Gesture::kDoubleClick:
      return source_.nextWarningOrDanger(config, pressure) && session_.updatePressure(pressure);
    case Gesture::kLongPress:
      session_.advancePage();
      return true;
  }
  return false;
}

}  // namespace gauge_core
