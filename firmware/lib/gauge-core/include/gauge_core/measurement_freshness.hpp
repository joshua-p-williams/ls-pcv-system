#pragma once

#include <cstdint>

#include "gauge_core/gauge_types.hpp"

namespace gauge_core {

/// Make a presentation copy stale after its configured freshness window.
///
/// The acquisition snapshot remains unchanged. This lets the UI suppress an
/// old live value without erasing useful extrema or trend history.
/// @param measurement Latest immutable acquisition state.
/// @param nowMs Current monotonic millisecond counter.
/// @param staleAfterMs Maximum age still considered fresh.
/// @return Copy with stale quality and no current pressure when expired.
MeasurementSnapshot withFreshnessPolicy(const MeasurementSnapshot& measurement,
                                        std::uint32_t nowMs,
                                        std::uint32_t staleAfterMs);

}  // namespace gauge_core
