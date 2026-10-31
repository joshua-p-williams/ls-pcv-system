#pragma once

#include "gauge_types.hpp"

namespace gauge_core {

class PressureClassifier {
 public:
  /// Classify a signed demo pressure using the shared threshold boundaries.
  /// @param pressure Pressure in tenths of inH2O.
  /// @param config Shared synthetic bands and visible span.
  /// @param result Receives the status only when this method returns true.
  /// @return False for an invalid configuration or a pressure outside its span.
  bool classify(PressureTenthsInH2O pressure, const DemoBandConfig& config,
                PressureBand& result) const;
};

}  // namespace gauge_core
