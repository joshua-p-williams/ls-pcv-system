#pragma once

#include "gauge_types.hpp"

namespace gauge_core {

/// Applies shared lower/upper pressure-band rules without hardware dependencies.
class PressureClassifier {
 public:
  /// Classify an already-valid pressure using the shared status boundaries.
  /// @param pressure Pressure in tenths of inH2O.
  /// @param thresholds Shared lower/upper warning and danger thresholds.
  /// @param result Receives the status only when this method returns true.
  /// @return False for invalid thresholds; sensor/domain validity is checked upstream.
  bool classify(PressureTenthsInH2O pressure,
                const PressureBandThresholds& thresholds,
                PressureBand& result) const;
};

}  // namespace gauge_core
