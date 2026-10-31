#pragma once

#include "demo_pressure_source.hpp"
#include "gauge_session.hpp"

namespace gauge_core {

// M01-only demo mapping. Future in-car gesture behavior is not established here.
class M01Interaction {
 public:
  /// Bind the demo source and the application session for one M01 interaction.
  /// @param source Seeded synthetic pressure source.
  /// @param session State updated by gestures.
  M01Interaction(DemoPressureSource& source, GaugeSession& session);

  /// Apply one already-translated gesture once.
  /// Click generates a normal value; double-click generates warning/danger;
  /// long press advances the page without changing pressure or history.
  /// @param gesture M01 gesture event.
  /// @return True when the gesture was handled; false if generation/update failed.
  bool handle(Gesture gesture);

 private:
  DemoPressureSource& source_;
  GaugeSession& session_;
};

}  // namespace gauge_core
