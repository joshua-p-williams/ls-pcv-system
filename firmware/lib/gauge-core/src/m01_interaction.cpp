#include "gauge_core/m01_interaction.hpp"

namespace gauge_core {

/// Bind the synthetic source and state object; neither is owned by this mapper.
M01Interaction::M01Interaction(DemoPressureSource& source, GaugeSession& session)
    : source_(source), session_(session) {}

/// Apply one temporary demo action. Long press changes the page only; timed
/// sampling is performed by GaugeSession::sampleAt(), not by this event handler.
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
