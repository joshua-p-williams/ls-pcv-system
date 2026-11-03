#pragma once

#include <LovyanGFX.hpp>
#include <lvgl.h>

#include <cstddef>
#include <cstdint>

#include "gauge_core/gauge_types.hpp"

namespace ls_pcv {
namespace display {

/// N16R8 GC9A01 display adapter.
/// LovyanGFX configures and transfers pixels to the SPI panel; LVGL owns widgets,
/// pages, layout, and animation. This adapter translates read-only session
/// snapshots into those widgets and never changes measurement/session state.
class LvglGc9a01 {
 public:
  /// Initialize the SPI panel, LVGL display, and three measurement pages.
  /// @param bands The shared synthetic span/threshold configuration used by the UI.
  /// @return True when the panel and LVGL display were initialized.
  bool begin(const gauge_core::DemoBandConfig& bands);

  /// Render the latest gauge/session values when initialization succeeded.
  /// @param snapshot Current live or synthetic state; history is chronological and may be partial.
  void render(const gauge_core::GaugeDisplaySnapshot& snapshot);

  /// Service LVGL timers and animations; call frequently from the owning loop task.
  void service();

 private:
  /// Bound tick-label storage if a future scale configuration becomes unusually dense.
  static constexpr std::size_t kMaxGaugeTicks = 256;

  /// Create the gauge, trend, and min/max LVGL object trees after display setup.
  void createPages();
  /// Refresh the gauge value, needle, status, and status-colored value hub.
  void updateGauge(const gauge_core::GaugeDisplaySnapshot& snapshot);
  /// Refresh the trend header and plot, leaving breaks where history is invalid.
  void updateTrend(const gauge_core::GaugeDisplaySnapshot& snapshot);
  /// Refresh current-run extrema; no values are inferred from the display itself.
  void updateMinMax(const gauge_core::GaugeDisplaySnapshot& snapshot);
  /// Animate from the current needle position to a new signed pressure value.
  void animateNeedleTo(gauge_core::PressureTenthsInH2O pressure);
  /// Apply one animation frame by setting LVGL's scale-needle value.
  void setNeedleValue(std::int32_t pressure);

  /// Keeps the concrete LovyanGFX SPI/panel details private to this adapter.
  class Panel;
  /// Heap-owned because the panel and bus object exceed a practical setup stack.
  Panel* panel_ = nullptr;
  /// LVGL display, page roots, child widgets, and persistent styles.
  lv_display_t* lvDisplay_ = nullptr;
  lv_obj_t* gaugePage_ = nullptr;
  lv_obj_t* trendPage_ = nullptr;
  lv_obj_t* minMaxPage_ = nullptr;
  lv_obj_t* trendValuePanel_ = nullptr;
  lv_obj_t* trendValueLabel_ = nullptr;
  lv_obj_t* trendMetaLabel_ = nullptr;
  lv_obj_t* scale_ = nullptr;
  lv_obj_t* needle_ = nullptr;
  lv_obj_t* valuePanel_ = nullptr;
  lv_obj_t* valueLabel_ = nullptr;
  lv_obj_t* unitLabel_ = nullptr;
  lv_obj_t* statusPanel_ = nullptr;
  lv_obj_t* statusLabel_ = nullptr;
  /// One line object per neighboring sample pair allows invalid intervals to be gaps.
  lv_obj_t* trendSegments_[gauge_core::kHistoryCapacity - 1]{};
  lv_point_precise_t trendSegmentPoints_[gauge_core::kHistoryCapacity - 1][2]{};
  lv_obj_t* minimumLabel_ = nullptr;
  lv_obj_t* maximumLabel_ = nullptr;
  lv_obj_t* minMaxStateLabel_ = nullptr;
  lv_obj_t* pageLabels_[3]{};
  lv_style_t dangerStyle_{};
  lv_style_t warningStyle_{};
  lv_style_t normalStyle_{};
  char gaugeTickText_[kMaxGaugeTicks][8]{};
  const char* gaugeTickLabels_[kMaxGaugeTicks + 1]{};
  /// Partial-render buffer: LVGL flushes rectangular regions through LovyanGFX.
  std::uint16_t drawBuffer_[gauge_core::kHistoryCapacity * 240]{};
  bool ready_ = false;
  gauge_core::DemoBandConfig bands_{};
  std::int32_t animatedNeedleValue_ = 0;
  std::int32_t lastNeedleTarget_ = 0;
  bool hasNeedleTarget_ = false;
};

}  // namespace display
}  // namespace ls_pcv
