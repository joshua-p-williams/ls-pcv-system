#include "display/lvgl_gc9a01.hpp"

#include <Arduino.h>
#include <cstdio>

#include "board/n16r8/board_config.hpp"

namespace ls_pcv {
namespace display {
namespace {

// Visual design tokens shared by all pages. These colors describe M01's
// synthetic bands only; later sensor thresholds must continue to come from
// the calibrated application configuration, not from these RGB values.
constexpr std::uint32_t kBackground = 0x090D12;
constexpr std::uint32_t kNormal = 0x147D45;
constexpr std::uint32_t kWarning = 0xD99A00;
constexpr std::uint32_t kDanger = 0xD93025;
constexpr std::uint32_t kWhite = 0xF3F6F8;
constexpr std::uint32_t kMuted = 0xAEBBC6;
constexpr std::uint32_t kDarkHub = 0x101820;
constexpr std::uint32_t kTrendLine = 0x38BDF8;
constexpr std::uint32_t kNeedleAnimationMs = 180;
constexpr std::uint32_t kGaugeNeedleLength = 90;
constexpr std::uint32_t kScaleAngleRange = 270;

/// LVGL needs a monotonic millisecond clock for animations and timer scheduling.
/// Arduino's millis() supplies that clock and wraps using unsigned arithmetic.
std::uint32_t lvglTick() { return millis(); }

// LovyanGFX objects are created during begin(); LVGL's C-style flush callback
// has no user context parameter, so this pointer routes flushes to the one panel.
// The firmware currently creates one display adapter for one physical screen.
lgfx::LGFX_Device* activePanel = nullptr;
void flushToPanel(lv_display_t* display, const lv_area_t* area,
                  std::uint8_t* pixels);

/// Convert the shared status enum to the short labels used by the display.
const char* bandName(gauge_core::PressureBand band) {
  switch (band) {
    case gauge_core::PressureBand::kDanger: return "DANGER";
    case gauge_core::PressureBand::kWarning: return "WARNING";
    case gauge_core::PressureBand::kNormal: return "NORMAL";
  }
  return "UNKNOWN";
}

/// Select a presentation color; the enum classification remains in gauge-core.
std::uint32_t bandColor(gauge_core::PressureBand band) {
  switch (band) {
    case gauge_core::PressureBand::kDanger: return kDanger;
    case gauge_core::PressureBand::kWarning: return kWarning;
    case gauge_core::PressureBand::kNormal: return kNormal;
  }
  return kMuted;
}

/// Add one inclusive colored range to the LVGL scale.
/// LVGL sections style arc, tick items, and indicator consistently so their
/// transition points align with the classifier's integer pressure boundaries.
void addSection(lv_obj_t* scale, std::int32_t first, std::int32_t last,
                lv_style_t* style) {
  lv_scale_section_t* section = lv_scale_add_section(scale);
  lv_scale_section_set_range(section, first, last);
  lv_scale_section_set_style(section, LV_PART_MAIN, style);
  lv_scale_section_set_style(section, LV_PART_ITEMS, style);
  lv_scale_section_set_style(section, LV_PART_INDICATOR, style);
}

/// Euclidean algorithm used to find exact tick spacing shared by all boundaries.
std::int32_t greatestCommonDivisor(std::int32_t left, std::int32_t right) {
  while (right != 0) {
    const std::int32_t remainder = left % right;
    left = right;
    right = remainder;
  }
  return left;
}

/// Find the largest evenly spaced tick interval that lands on every transition.
/// Inputs use tenths of inH2O, so the current span supports half-unit ticks.
std::int32_t exactGaugeTickStep(const gauge_core::DemoBandConfig& bands) {
  // Choose the greatest evenly spaced step that divides the whole span and
  // every threshold offset. Each threshold then has a corresponding LVGL tick.
  const std::int32_t span = bands.maximum - bands.minimum;
  std::int32_t step = span;
  step = greatestCommonDivisor(step, bands.lowerDanger - bands.minimum);
  step = greatestCommonDivisor(step, bands.lowerWarning - bands.minimum);
  step = greatestCommonDivisor(step, bands.upperWarning - bands.minimum);
  step = greatestCommonDivisor(step, bands.upperDanger - bands.minimum);
  return step;
}

/// Show whole-unit labels while leaving half-unit marks as unlabeled ticks.
bool isGaugeLabelValue(std::int32_t pressure) {
  // Label each whole inH2O value. The +4.5 scale endpoint remains available
  // for the needle and colored band but is intentionally left unlabeled.
  return pressure % 10 == 0;
}

/// Format a scale value from tenths, retaining one decimal only when needed.
void formatGaugeLabel(char* destination, std::size_t capacity,
                      std::int32_t tenths) {
  if (tenths == 0) {
    std::snprintf(destination, capacity, "0");
  } else if (tenths % 10 == 0) {
    std::snprintf(destination, capacity, "%+ld", static_cast<long>(tenths / 10));
  } else {
    std::snprintf(destination, capacity, "%+.1f",
                  static_cast<double>(tenths) / 10.0);
  }
}

/// Create a label using the display's shared centered layout and text styling.
lv_obj_t* createCenteredLabel(lv_obj_t* parent, const char* text,
                              const lv_font_t* font, std::uint32_t color,
                              std::int32_t y) {
  lv_obj_t* label = lv_label_create(parent);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_color(label, lv_color_hex(color), LV_PART_MAIN);
  lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, y);
  return label;
}

}  // namespace

/// LovyanGFX panel using the W01 GPIO assignment, not library default pins.
class LvglGc9a01::Panel : public lgfx::LGFX_Device {
 public:
  /// Configure the SPI bus and GC9A01 panel from the N16R8 W01 pin aliases.
  /// LovyanGFX separates bus pins (clock/data/DC) from panel pins (CS/reset),
  /// then owns the low-level commands and pixel transfers for LVGL.
  Panel() {
    auto busConfig = bus_.config();
    busConfig.spi_host = SPI2_HOST;
    busConfig.spi_mode = 0;
    // These write/read rates and SPI mode come from the physical N16R8/GC9A01
    // proof. The module is configured write-only below, so no MISO GPIO is used.
    busConfig.freq_write = 20000000;
    busConfig.freq_read = 10000000;
    busConfig.spi_3wire = false;
    busConfig.use_lock = true;
    busConfig.dma_channel = SPI_DMA_CH_AUTO;
    busConfig.pin_sclk = ls_pcv::board::n16r8::kDisplayClockPin;
    busConfig.pin_mosi = ls_pcv::board::n16r8::kDisplayMosiPin;
    busConfig.pin_miso = -1;
    busConfig.pin_dc = ls_pcv::board::n16r8::kDisplayDcPin;
    bus_.config(busConfig);
    panel_.setBus(&bus_);

    auto panelConfig = panel_.config();
    panelConfig.pin_cs = ls_pcv::board::n16r8::kDisplayChipSelectPin;
    panelConfig.pin_rst = ls_pcv::board::n16r8::kDisplayResetPin;
    panelConfig.pin_busy = -1;
    panelConfig.panel_width = ls_pcv::board::n16r8::kDisplayWidth;
    panelConfig.panel_height = ls_pcv::board::n16r8::kDisplayHeight;
    panelConfig.offset_x = 0;
    panelConfig.offset_y = 0;
    panelConfig.offset_rotation = 0;
    panelConfig.readable = false;
    panelConfig.invert = true;
    panelConfig.rgb_order = false;
    panelConfig.dlen_16bit = false;
    panelConfig.bus_shared = false;
    panel_.config(panelConfig);
    setPanel(&panel_);
  }

 private:
  lgfx::Bus_SPI bus_;
  lgfx::Panel_GC9A01 panel_;
};

namespace {
/// LVGL callback that transfers one rendered rectangle to the active SPI panel.
/// flush_ready() tells LVGL its borrowed draw-buffer region can be reused; this
/// implementation is synchronous and therefore contributes to loop-task latency.
void flushToPanel(lv_display_t* display, const lv_area_t* area,
                  std::uint8_t* pixels) {
  if (activePanel == nullptr) {
    lv_display_flush_ready(display);
    return;
  }
  const std::int32_t width = area->x2 - area->x1 + 1;
  const std::int32_t height = area->y2 - area->y1 + 1;
  activePanel->pushImage(area->x1, area->y1, width, height,
                         reinterpret_cast<const std::uint16_t*>(pixels));
  lv_display_flush_ready(display);
}
}  // namespace

/// Allocate and initialize the hardware/display stack before creating widgets.
/// This method returns failure rather than waiting for a USB console or retrying
/// forever; the caller can keep the rest of firmware running without a display.
bool LvglGc9a01::begin(const gauge_core::DemoBandConfig& bands) {
  if (ready_) return true;
  if (!bands.isValid()) return false;
  const std::int32_t tickStep = exactGaugeTickStep(bands);
  const std::int32_t span = bands.maximum - bands.minimum;
  const std::size_t tickCount = static_cast<std::size_t>(span / tickStep) + 1;
  if (tickStep <= 0 || tickCount > kMaxGaugeTicks) return false;
  bands_ = bands;

  // Keep the large panel object off the task stack. Its lifetime is the adapter's.
  panel_ = new (std::nothrow) Panel();
  if (panel_ == nullptr || !panel_->init()) {
    delete panel_;
    panel_ = nullptr;
    return false;
  }
  panel_->setColorDepth(16);
  // LVGL's RGB565 draw buffer is little-endian; the tested panel path swaps words.
  panel_->setSwapBytes(true);
  panel_->setRotation(0);
  panel_->fillScreen(TFT_BLACK);

  activePanel = panel_;
  lv_init();
  lv_tick_set_cb(lvglTick);
  lvDisplay_ = lv_display_create(ls_pcv::board::n16r8::kDisplayWidth,
                                 ls_pcv::board::n16r8::kDisplayHeight);
  if (lvDisplay_ == nullptr) return false;
  lv_display_set_color_format(lvDisplay_, LV_COLOR_FORMAT_RGB565);
  lv_display_set_flush_cb(lvDisplay_, flushToPanel);
  // Partial mode reuses this bounded strip buffer instead of allocating a full
  // 240 x 240 framebuffer; LVGL requests dirty rectangles through flushToPanel.
  lv_display_set_buffers(lvDisplay_, drawBuffer_, nullptr, sizeof(drawBuffer_),
                         LV_DISPLAY_RENDER_MODE_PARTIAL);
  createPages();
  ready_ = true;
  return true;
}

/// Build all three LVGL page trees once; later calls only update their widgets.
/// Keeping pressure state outside LVGL lets host-tested core logic and rendering
/// remain separate responsibilities.
void LvglGc9a01::createPages() {
  gaugePage_ = lv_obj_create(nullptr);
  lv_obj_set_style_bg_color(gaugePage_, lv_color_hex(kBackground), LV_PART_MAIN);
  scale_ = lv_scale_create(gaugePage_);
  lv_obj_set_size(scale_, 224, 224);
  lv_obj_align(scale_, LV_ALIGN_CENTER, 0, 0);
  lv_scale_set_mode(scale_, LV_SCALE_MODE_ROUND_INNER);
  lv_scale_set_range(scale_, bands_.minimum, bands_.maximum);
  lv_scale_set_angle_range(scale_, kScaleAngleRange);
  lv_scale_set_rotation(scale_, 135);
  const std::int32_t tickStep = exactGaugeTickStep(bands_);
  const std::size_t tickCount = static_cast<std::size_t>(
      (bands_.maximum - bands_.minimum) / tickStep) + 1;
  lv_scale_set_total_tick_count(scale_, tickCount);
  // Every calculated half-unit tick is a major LVGL tick. The label source
  // below deliberately leaves half-unit labels empty to keep the dial legible.
  lv_scale_set_major_tick_every(scale_, 1);
  lv_scale_set_label_show(scale_, true);
  for (std::size_t index = 0; index < tickCount; ++index) {
    const std::int32_t tickPressure = bands_.minimum +
        static_cast<std::int32_t>(index) * tickStep;
    gaugeTickLabels_[index] = "";
    if (isGaugeLabelValue(tickPressure)) {
      formatGaugeLabel(gaugeTickText_[index], sizeof(gaugeTickText_[index]),
                       tickPressure);
      gaugeTickLabels_[index] = gaugeTickText_[index];
    }
  }
  gaugeTickLabels_[tickCount] = nullptr;
  lv_scale_set_text_src(scale_, gaugeTickLabels_);
  lv_obj_set_style_arc_color(scale_, lv_color_hex(0x26313B), LV_PART_MAIN);
  lv_obj_set_style_arc_width(scale_, 7, LV_PART_MAIN);
  lv_obj_set_style_line_color(scale_, lv_color_hex(kWhite), LV_PART_ITEMS);
  lv_obj_set_style_line_width(scale_, 1, LV_PART_ITEMS);
  lv_obj_set_style_length(scale_, 4, LV_PART_ITEMS);
  lv_obj_set_style_line_color(scale_, lv_color_hex(kWhite), LV_PART_INDICATOR);
  lv_obj_set_style_line_width(scale_, 2, LV_PART_INDICATOR);
  lv_obj_set_style_length(scale_, 4, LV_PART_INDICATOR);
  lv_obj_set_style_text_color(scale_, lv_color_hex(kWhite), LV_PART_INDICATOR);
  lv_obj_set_style_text_font(scale_, &lv_font_montserrat_10, LV_PART_INDICATOR);

  lv_style_init(&dangerStyle_);
  lv_style_init(&warningStyle_);
  lv_style_init(&normalStyle_);
  const std::uint32_t sectionColors[] = {kDanger, kWarning, kNormal};
  lv_style_t* styles[] = {&dangerStyle_, &warningStyle_, &normalStyle_};
  for (std::size_t i = 0; i < 3; ++i) {
    lv_style_set_arc_color(styles[i], lv_color_hex(sectionColors[i]));
    lv_style_set_arc_width(styles[i], 7);
    lv_style_set_line_color(styles[i], lv_color_hex(sectionColors[i]));
    lv_style_set_line_width(styles[i], 2);
    lv_style_set_text_color(styles[i], lv_color_hex(sectionColors[i]));
  }

  // LVGL scale coordinates are integers; passing tenths directly preserves
  // half-unit demo marks (for example, -8.0 inH2O is -80). The needle, sections,
  // and classifier therefore all use one shared integer configuration.
  // Each status interval is split at one tenth because the classifier's
  // boundary rules assign the exact transition value to one neighboring band.
  addSection(scale_, bands_.minimum, bands_.lowerDanger - 1, &dangerStyle_);
  addSection(scale_, bands_.lowerDanger, bands_.lowerWarning - 1, &warningStyle_);
  addSection(scale_, bands_.lowerWarning, bands_.upperWarning, &normalStyle_);
  addSection(scale_, bands_.upperWarning + 1, bands_.upperDanger, &warningStyle_);
  addSection(scale_, bands_.upperDanger + 1, bands_.maximum, &dangerStyle_);
  needle_ = lv_line_create(scale_);
  lv_obj_set_style_line_color(needle_, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_line_width(needle_, 3, LV_PART_MAIN);
  lv_obj_set_style_line_rounded(needle_, true, LV_PART_MAIN);
  valuePanel_ = lv_obj_create(gaugePage_);
  lv_obj_set_size(valuePanel_, 82, 82);
  lv_obj_align(valuePanel_, LV_ALIGN_CENTER, 0, -2);
  lv_obj_set_style_bg_color(valuePanel_, lv_color_hex(kDarkHub), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(valuePanel_, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_width(valuePanel_, 0, LV_PART_MAIN);
  lv_obj_set_style_radius(valuePanel_, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_set_style_pad_all(valuePanel_, 0, LV_PART_MAIN);
  valueLabel_ = createCenteredLabel(gaugePage_, "--", &lv_font_montserrat_32,
                                   kWhite, -12);
  unitLabel_ = createCenteredLabel(gaugePage_, "inH2O", &lv_font_montserrat_14,
                                   kMuted, 16);
  statusPanel_ = lv_obj_create(gaugePage_);
  lv_obj_set_size(statusPanel_, 94, 25);
  lv_obj_align(statusPanel_, LV_ALIGN_CENTER, 0, 55);
  lv_obj_set_style_radius(statusPanel_, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_set_style_border_width(statusPanel_, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(statusPanel_, 2, LV_PART_MAIN);
  statusLabel_ = lv_label_create(statusPanel_);
  lv_obj_set_style_text_color(statusLabel_, lv_color_hex(kWhite), LV_PART_MAIN);
  lv_obj_set_style_text_font(statusLabel_, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_obj_center(statusLabel_);
  pageLabels_[0] = createCenteredLabel(gaugePage_, "GAUGE  1/3",
                                      &lv_font_montserrat_14, kMuted, 104);

  trendPage_ = lv_obj_create(nullptr);
  lv_obj_set_style_bg_color(trendPage_, lv_color_hex(kBackground), LV_PART_MAIN);
  trendValuePanel_ = lv_obj_create(trendPage_);
  lv_obj_set_size(trendValuePanel_, 150, 30);
  lv_obj_align(trendValuePanel_, LV_ALIGN_CENTER, 0, -96);
  lv_obj_set_style_radius(trendValuePanel_, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_set_style_border_width(trendValuePanel_, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(trendValuePanel_, 2, LV_PART_MAIN);
  trendValueLabel_ = lv_label_create(trendValuePanel_);
  lv_obj_set_style_text_color(trendValueLabel_, lv_color_hex(kWhite), LV_PART_MAIN);
  lv_obj_set_style_text_font(trendValueLabel_, &lv_font_montserrat_20,
                             LV_PART_MAIN);
  lv_obj_center(trendValueLabel_);
  trendMetaLabel_ = createCenteredLabel(trendPage_, "NORMAL | 1s | 0/30",
                                        &lv_font_montserrat_14, kMuted, -70);
  lv_obj_t* plot = lv_obj_create(trendPage_);
  lv_obj_set_size(plot, 208, 130);
  lv_obj_align(plot, LV_ALIGN_CENTER, 0, 5);
  lv_obj_set_style_bg_color(plot, lv_color_hex(0x111923), LV_PART_MAIN);
  lv_obj_set_style_border_color(plot, lv_color_hex(0x43505C), LV_PART_MAIN);
  lv_obj_set_style_radius(plot, 4, LV_PART_MAIN);
  lv_obj_set_style_pad_all(plot, 0, LV_PART_MAIN);

  // Shade the five pressure bands behind the trace using the exact same source
  // values as the gauge ribbon and classifier. This prevents the graph's visual
  // background from presenting different state boundaries than the main gauge.
  constexpr std::int32_t kPlotTop = 8;
  constexpr std::int32_t kPlotBottom = 118;
  constexpr std::int32_t kPlotHeight = kPlotBottom - kPlotTop;
  constexpr std::int32_t kPlotLeft = 34;
  constexpr std::int32_t kPlotWidth = 168;
  const auto plotY = [this](std::int32_t pressure) {
    return kPlotBottom - (pressure - bands_.minimum) * kPlotHeight /
                              (bands_.maximum - bands_.minimum);
  };
  const auto addBand = [plot](std::int32_t top, std::int32_t bottom,
                              std::uint32_t color) {
    lv_obj_t* band = lv_obj_create(plot);
    lv_obj_set_size(band, kPlotWidth, bottom - top);
    lv_obj_align(band, LV_ALIGN_TOP_LEFT, kPlotLeft, top);
    lv_obj_set_style_bg_color(band, lv_color_hex(color), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(band, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_border_width(band, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(band, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(band, 0, LV_PART_MAIN);
  };
  addBand(kPlotTop, plotY(bands_.upperDanger), kDanger);
  addBand(plotY(bands_.upperDanger), plotY(bands_.upperWarning), kWarning);
  addBand(plotY(bands_.upperWarning), plotY(bands_.lowerWarning), kNormal);
  addBand(plotY(bands_.lowerWarning), plotY(bands_.lowerDanger), kWarning);
  addBand(plotY(bands_.lowerDanger), kPlotBottom, kDanger);

  lv_obj_t* zeroLine = lv_line_create(plot);
  const std::int32_t zeroY = plotY(0);
  lv_point_precise_t zeroPoints[] = {{kPlotLeft, zeroY},
                                     {kPlotLeft + kPlotWidth, zeroY}};
  lv_line_set_points(zeroLine, zeroPoints, 2);
  lv_obj_set_style_line_color(zeroLine, lv_color_hex(0x64748B), LV_PART_MAIN);
  lv_obj_set_style_line_width(zeroLine, 1, LV_PART_MAIN);

  // Numeric axis marks give the colored graph a readable pressure reference.
  const auto addAxisLabel = [plot, plotY](const char* text, std::int32_t pressure) {
    const std::int32_t y = plotY(pressure) - 6;
    lv_obj_t* label = lv_label_create(plot);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(kWhite), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_10, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 2, y);
  };
  addAxisLabel("+4.5", bands_.maximum);
  addAxisLabel("0", 0);
  addAxisLabel("-8", bands_.lowerWarning);
  addAxisLabel("-14", bands_.minimum);

  trendLine_ = lv_line_create(plot);
  lv_obj_set_style_line_color(trendLine_, lv_color_hex(kTrendLine), LV_PART_MAIN);
  lv_obj_set_style_line_width(trendLine_, 3, LV_PART_MAIN);
  lv_obj_set_style_line_rounded(trendLine_, true, LV_PART_MAIN);
  pageLabels_[1] = createCenteredLabel(trendPage_, "TREND  2/3",
                                      &lv_font_montserrat_14, kMuted, 105);

  minMaxPage_ = lv_obj_create(nullptr);
  lv_obj_set_style_bg_color(minMaxPage_, lv_color_hex(kBackground), LV_PART_MAIN);
  createCenteredLabel(minMaxPage_, "CURRENT RUN", &lv_font_montserrat_20,
                      kWhite, -86);
  createCenteredLabel(minMaxPage_, "MINIMUM", &lv_font_montserrat_14,
                      kMuted, -42);
  minimumLabel_ = createCenteredLabel(minMaxPage_, "--.- inH2O",
                                     &lv_font_montserrat_20, kTrendLine, -14);
  createCenteredLabel(minMaxPage_, "MAXIMUM", &lv_font_montserrat_14,
                      kMuted, 28);
  maximumLabel_ = createCenteredLabel(minMaxPage_, "--.- inH2O",
                                     &lv_font_montserrat_20, kWarning, 56);
  pageLabels_[2] = createCenteredLabel(minMaxPage_, "MIN / MAX  3/3",
                                      &lv_font_montserrat_14, kMuted, 105);
  lv_screen_load(gaugePage_);
}

void LvglGc9a01::render(const gauge_core::GaugeDisplaySnapshot& snapshot) {
  if (!ready_) return;
  updateGauge(snapshot);
  updateTrend(snapshot);
  updateMinMax(snapshot);
  lv_obj_t* page = gaugePage_;
  switch (snapshot.page) {
    case gauge_core::Page::kGauge: page = gaugePage_; break;
    case gauge_core::Page::kTrend: page = trendPage_; break;
    case gauge_core::Page::kMinMax: page = minMaxPage_; break;
  }
  if (lv_screen_active() != page) lv_screen_load_anim(page, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, false);
}

/// Update only the gauge widgets from the snapshot; do not classify a second time.
void LvglGc9a01::updateGauge(const gauge_core::GaugeDisplaySnapshot& snapshot) {
  if (!snapshot.hasPressure) return;
  animateNeedleTo(snapshot.pressure);
  char formatted[16];
  std::snprintf(formatted, sizeof(formatted), "%+.1f",
                static_cast<double>(snapshot.pressure) / 10.0);
  lv_label_set_text(valueLabel_, formatted);
  lv_label_set_text(statusLabel_, bandName(snapshot.band));
  const std::uint32_t color = bandColor(snapshot.band);
  const bool amber = snapshot.band == gauge_core::PressureBand::kWarning;
  const std::uint32_t textColor = amber ? kDarkHub : kWhite;
  const std::uint32_t hubColor = snapshot.band == gauge_core::PressureBand::kNormal
                                     ? kDarkHub : color;
  lv_obj_set_style_bg_color(valuePanel_, lv_color_hex(hubColor), LV_PART_MAIN);
  lv_obj_set_style_text_color(valueLabel_, lv_color_hex(textColor), LV_PART_MAIN);
  lv_obj_set_style_text_color(unitLabel_, lv_color_hex(textColor), LV_PART_MAIN);
  lv_obj_set_style_bg_color(statusPanel_, lv_color_hex(color), LV_PART_MAIN);
  lv_obj_set_style_text_color(statusLabel_, lv_color_hex(textColor), LV_PART_MAIN);
}

/// Convert recorded pressure samples to chart coordinates and refresh trend text.
/// The x-axis reserves its full 30-second width while partial history starts at
/// the left edge; absent startup samples are never fabricated or interpolated.
void LvglGc9a01::updateTrend(const gauge_core::GaugeDisplaySnapshot& snapshot) {
  char currentValue[24];
  std::snprintf(currentValue, sizeof(currentValue), "%+.1f inH2O",
                static_cast<double>(snapshot.pressure) / 10.0);
  lv_label_set_text(trendValueLabel_, currentValue);
  const bool warning = snapshot.band == gauge_core::PressureBand::kWarning;
  const std::uint32_t statusColor = bandColor(snapshot.band);
  lv_obj_set_style_bg_color(trendValuePanel_, lv_color_hex(statusColor), LV_PART_MAIN);
  lv_obj_set_style_text_color(trendValueLabel_,
      lv_color_hex(warning ? kDarkHub : kWhite), LV_PART_MAIN);
  char metadata[32];
  std::snprintf(metadata, sizeof(metadata), "%s | 1s | %u/30",
                bandName(snapshot.band), static_cast<unsigned>(snapshot.historyCount));
  lv_label_set_text(trendMetaLabel_, metadata);
  if (snapshot.historyCount < 2) {
    lv_obj_add_flag(trendLine_, LV_OBJ_FLAG_HIDDEN);
    return;
  }
  // A single LVGL line cannot represent a gap. Hide the trace rather than
  // drawing an invalid interval as pressure; M02's display phase can add
  // segmented line objects once live invalid samples are presented here.
  for (std::size_t i = 0; i < snapshot.historyCount; ++i) {
    if (!snapshot.history[i].valid) {
      lv_obj_add_flag(trendLine_, LV_OBJ_FLAG_HIDDEN);
      return;
    }
  }
  lv_obj_clear_flag(trendLine_, LV_OBJ_FLAG_HIDDEN);
  // Plot only collected points. The fixed vertical span is the same demo range;
  // partial startup history is left-aligned instead of padded with fake data.
  constexpr std::int32_t kPlotWidth = 168;
  constexpr std::int32_t kPlotHeight = 110;
  constexpr std::int32_t kPlotLeft = 34;
  constexpr std::int32_t kPlotTop = 8;
  const std::int32_t span = snapshot.bands.maximum - snapshot.bands.minimum;
  for (std::size_t i = 0; i < snapshot.historyCount; ++i) {
    const std::int32_t x = kPlotLeft +
        static_cast<std::int32_t>(i * kPlotWidth /
            (gauge_core::kHistoryCapacity - 1));
    const std::int32_t pressure = snapshot.history[i].pressure;
    const std::int32_t bounded = pressure < snapshot.bands.minimum
        ? snapshot.bands.minimum
        : (pressure > snapshot.bands.maximum
              ? snapshot.bands.maximum : pressure);
    const std::int32_t y = kPlotTop + kPlotHeight -
        static_cast<std::int32_t>((bounded - snapshot.bands.minimum) * kPlotHeight / span);
    trendPoints_[i] = {static_cast<lv_value_precise_t>(x),
                       static_cast<lv_value_precise_t>(y)};
  }
  lv_line_set_points(trendLine_, trendPoints_, snapshot.historyCount);
}

/// Display extrema already captured by GaugeSession, without resetting on page entry.
void LvglGc9a01::updateMinMax(const gauge_core::GaugeDisplaySnapshot& snapshot) {
  if (!snapshot.hasExtrema) return;
  char formatted[24];
  std::snprintf(formatted, sizeof(formatted), "%+.1f inH2O",
                static_cast<double>(snapshot.minimumRecorded) / 10.0);
  lv_label_set_text(minimumLabel_, formatted);
  std::snprintf(formatted, sizeof(formatted), "%+.1f inH2O",
                static_cast<double>(snapshot.maximumRecorded) / 10.0);
  lv_label_set_text(maximumLabel_, formatted);
}

/// Ask LVGL to interpolate the needle position; service() advances the animation.
void LvglGc9a01::animateNeedleTo(gauge_core::PressureTenthsInH2O pressure) {
  if (hasNeedleTarget_ && pressure == lastNeedleTarget_) return;
  hasNeedleTarget_ = true;
  lastNeedleTarget_ = pressure;
  lv_anim_t animation;
  lv_anim_init(&animation);
  lv_anim_set_var(&animation, this);
  lv_anim_set_values(&animation, animatedNeedleValue_, pressure);
  lv_anim_set_duration(&animation, kNeedleAnimationMs);
  lv_anim_set_exec_cb(&animation, [](void* object, std::int32_t value) {
    static_cast<LvglGc9a01*>(object)->setNeedleValue(value);
  });
  lv_anim_start(&animation);
}

/// Convert the current animated pressure back into a position on the LVGL scale.
void LvglGc9a01::setNeedleValue(std::int32_t pressure) {
  animatedNeedleValue_ = pressure;
  lv_scale_set_line_needle_value(scale_, needle_, kGaugeNeedleLength, pressure);
}

/// Run LVGL timers, pending refreshes, and animation callbacks from loop().
/// This must stay in the same task/context as render(); the button poller only
/// sends gesture events and never calls into LVGL directly.
void LvglGc9a01::service() {
  if (ready_) lv_timer_handler();
}

}  // namespace display
}  // namespace ls_pcv
