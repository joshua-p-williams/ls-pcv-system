#pragma once

#include <LovyanGFX.hpp>
#include <lvgl.h>

#include <cstddef>
#include <cstdint>

#include "gauge_core/gauge_types.hpp"

namespace ls_pcv {
namespace display {

/// N16R8 GC9A01 display adapter: owns panel setup and renders read-only M01 snapshots.
class LvglGc9a01 {
 public:
  /// Initialize the SPI panel and LVGL display/pages without waiting for USB.
  /// @return True when the panel and LVGL display were initialized.
  bool begin(const gauge_core::DemoBandConfig& bands);

  /// Render the latest gauge/session values when initialization succeeded.
  /// @param snapshot Current M01 state; history is chronological and may be partial.
  void render(const gauge_core::DisplaySnapshot& snapshot);

  /// Service LVGL timers and animations; call frequently from the main loop.
  void service();

 private:
  // M01's signed display span is small; bound storage if future settings make
  // exact boundary ticks impractically dense.
  static constexpr std::size_t kMaxGaugeTicks = 256;

  void createPages();
  void updateGauge(const gauge_core::DisplaySnapshot& snapshot);
  void updateTrend(const gauge_core::DisplaySnapshot& snapshot);
  void updateMinMax(const gauge_core::DisplaySnapshot& snapshot);
  void animateNeedleTo(gauge_core::PressureTenthsInH2O pressure);
  void setNeedleValue(std::int32_t pressure);

  class Panel;
  Panel* panel_ = nullptr;
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
  lv_obj_t* trendLine_ = nullptr;
  lv_point_precise_t trendPoints_[gauge_core::kHistoryCapacity]{};
  lv_obj_t* minimumLabel_ = nullptr;
  lv_obj_t* maximumLabel_ = nullptr;
  lv_obj_t* pageLabels_[3]{};
  lv_style_t dangerStyle_{};
  lv_style_t warningStyle_{};
  lv_style_t normalStyle_{};
  char gaugeTickText_[kMaxGaugeTicks][8]{};
  const char* gaugeTickLabels_[kMaxGaugeTicks + 1]{};
  std::uint16_t drawBuffer_[gauge_core::kHistoryCapacity * 240]{};
  bool ready_ = false;
  gauge_core::DemoBandConfig bands_{};
  std::int32_t animatedNeedleValue_ = 0;
  std::int32_t lastNeedleTarget_ = 0;
  bool hasNeedleTarget_ = false;
};

}  // namespace display
}  // namespace ls_pcv
