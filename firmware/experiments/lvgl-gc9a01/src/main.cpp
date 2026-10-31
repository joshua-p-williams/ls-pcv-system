#include <Arduino.h>
#include <lvgl.h>

#include "lvgl_gc9a01.hpp"

namespace {

constexpr int32_t kGaugeMinimum = -20;
constexpr int32_t kGaugeMaximum = 10;
constexpr int32_t kLowerDanger = -16;
constexpr int32_t kLowerWarning = -8;
constexpr int32_t kUpperWarning = -3;
constexpr int32_t kUpperDanger = 0;
constexpr uint32_t kSampleIntervalMs = 180;
constexpr uint32_t kBufferRows = 32;

constexpr int kDisplayWidth = 240;
constexpr int kDisplayHeight = 240;

LGFX display;
lv_display_t* lv_display = nullptr;
lv_obj_t* scale = nullptr;
lv_obj_t* needle = nullptr;
lv_obj_t* value_panel = nullptr;
lv_obj_t* value_label = nullptr;
lv_obj_t* unit_label = nullptr;
lv_obj_t* status_label = nullptr;
lv_obj_t* status_panel = nullptr;

// Partial buffers limit internal RAM use: each RGB565 buffer covers 32 rows.
uint16_t draw_buffer[kDisplayWidth * kBufferRows];

lv_style_t danger_style;
lv_style_t warning_style;
lv_style_t normal_style;

int32_t current_demo_value = -18;
int32_t demo_direction = 1;
uint32_t last_sample_ms = 0;

uint32_t lvgl_tick_ms() { return millis(); }

void flush_to_gc9a01(lv_display_t* display_context, const lv_area_t* area,
                     uint8_t* pixel_map) {
  const int32_t width = area->x2 - area->x1 + 1;
  const int32_t height = area->y2 - area->y1 + 1;
  display.pushImage(area->x1, area->y1, width, height,
                    reinterpret_cast<const uint16_t*>(pixel_map));
  lv_display_flush_ready(display_context);
}

void initialize_section_style(lv_style_t* style, uint32_t color) {
  lv_style_init(style);
  lv_style_set_arc_color(style, lv_color_hex(color));
  lv_style_set_arc_width(style, 7);
  lv_style_set_line_color(style, lv_color_hex(color));
  lv_style_set_line_width(style, 2);
  lv_style_set_text_color(style, lv_color_hex(color));
}

void add_scale_section(int32_t lower, int32_t upper, lv_style_t* style) {
  lv_scale_section_t* section = lv_scale_add_section(scale);
  lv_scale_section_set_range(section, lower, upper);
  lv_scale_section_set_style(section, LV_PART_MAIN, style);
  lv_scale_section_set_style(section, LV_PART_ITEMS, style);
  lv_scale_section_set_style(section, LV_PART_INDICATOR, style);
}

const char* status_for(int32_t value) {
  if (value < kLowerDanger || value > kUpperDanger) {
    return "DANGER";
  }
  if (value < kLowerWarning || value > kUpperWarning) {
    return "WARNING";
  }
  return "NORMAL";
}

uint32_t status_color_for(int32_t value) {
  if (value < kLowerDanger || value > kUpperDanger) {
    return 0xD93025;  // Red: demo danger band.
  }
  if (value < kLowerWarning || value > kUpperWarning) {
    return 0xD99A00;  // Amber: demo warning band.
  }
  return 0x147D45;    // Green: demo normal band.
}

uint32_t value_panel_color_for(int32_t value) {
  if (value < kLowerDanger || value > kUpperDanger) {
    return 0xD93025;  // Red center hub when the demo enters danger.
  }
  if (value < kLowerWarning || value > kUpperWarning) {
    return 0xD99A00;  // Amber center hub in either warning band.
  }
  return 0x101820;    // Keep the value hub dark in the normal band.
}

uint32_t value_text_color_for(int32_t value) {
  // Dark text is easier to read on amber; red and the normal dark hub use white.
  if ((value >= kLowerDanger && value < kLowerWarning) ||
      (value > kUpperWarning && value <= kUpperDanger)) {
    return 0x101820;
  }
  return 0xFFFFFF;
}

uint32_t status_text_color_for(int32_t value) {
  // Use dark text on the amber status pill for stronger contrast.
  if ((value >= kLowerDanger && value < kLowerWarning) ||
      (value > kUpperWarning && value <= kUpperDanger)) {
    return 0x101820;
  }
  return 0xFFFFFF;
}

void update_demo_value(int32_t value) {
  current_demo_value = value;
  lv_scale_set_line_needle_value(scale, needle, 90, value);

  const uint32_t panel_color = value_panel_color_for(value);
  const uint32_t text_color = value_text_color_for(value);
  lv_obj_set_style_bg_color(value_panel, lv_color_hex(panel_color),
                            LV_PART_MAIN);
  lv_obj_set_style_text_color(value_label, lv_color_hex(text_color),
                              LV_PART_MAIN);
  lv_obj_set_style_text_color(unit_label, lv_color_hex(text_color),
                              LV_PART_MAIN);

  char formatted_value[20];
  snprintf(formatted_value, sizeof(formatted_value), "%+ld",
           static_cast<long>(value));
  lv_label_set_text(value_label, formatted_value);
  lv_obj_set_style_bg_color(status_panel,
                            lv_color_hex(status_color_for(value)),
                            LV_PART_MAIN);
  lv_obj_set_style_text_color(status_label,
                              lv_color_hex(status_text_color_for(value)),
                              LV_PART_MAIN);
  lv_label_set_text(status_label, status_for(value));
}

void create_gauge_screen() {
  lv_obj_t* screen = lv_screen_active();
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x090D12), LV_PART_MAIN);

  scale = lv_scale_create(screen);
  lv_obj_set_size(scale, 224, 224);
  lv_obj_align(scale, LV_ALIGN_CENTER, 0, 0);
  lv_scale_set_mode(scale, LV_SCALE_MODE_ROUND_INNER);
  lv_scale_set_range(scale, kGaugeMinimum, kGaugeMaximum);
  lv_scale_set_angle_range(scale, 270);
  lv_scale_set_rotation(scale, 135);
  lv_scale_set_total_tick_count(scale, 31);
  lv_scale_set_major_tick_every(scale, 5);
  lv_scale_set_label_show(scale, true);
  lv_obj_set_style_arc_color(scale, lv_color_hex(0x26313B), LV_PART_MAIN);
  lv_obj_set_style_arc_width(scale, 7, LV_PART_MAIN);
  lv_obj_set_style_line_color(scale, lv_color_hex(0xDDE5EC), LV_PART_ITEMS);
  lv_obj_set_style_line_width(scale, 1, LV_PART_ITEMS);
  lv_obj_set_style_length(scale, 5, LV_PART_ITEMS);
  lv_obj_set_style_line_color(scale, lv_color_hex(0xF3F6F8),
                              LV_PART_INDICATOR);
  lv_obj_set_style_line_width(scale, 2, LV_PART_INDICATOR);
  lv_obj_set_style_length(scale, 9, LV_PART_INDICATOR);
  lv_obj_set_style_text_color(scale, lv_color_hex(0xF3F6F8),
                              LV_PART_INDICATOR);
  lv_obj_set_style_text_font(scale, &lv_font_montserrat_14,
                             LV_PART_INDICATOR);

  initialize_section_style(&danger_style, 0xD93025);
  initialize_section_style(&warning_style, 0xD99A00);
  initialize_section_style(&normal_style, 0x24B36B);
  add_scale_section(kGaugeMinimum, kLowerDanger, &danger_style);
  add_scale_section(kLowerDanger, kLowerWarning, &warning_style);
  add_scale_section(kLowerWarning, kUpperWarning, &normal_style);
  add_scale_section(kUpperWarning, kUpperDanger, &warning_style);
  add_scale_section(kUpperDanger, kGaugeMaximum, &danger_style);

  // LVGL positions a scale needle relative to the scale's top-left corner.
  // Making it a scale child keeps its pivot concentric when the scale is
  // centered with an inset from the full display bounds.
  needle = lv_line_create(scale);
  lv_obj_set_style_line_color(needle, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_line_width(needle, 3, LV_PART_MAIN);
  lv_obj_set_style_line_rounded(needle, true, LV_PART_MAIN);

  // The compact opaque hub masks the needle only beneath the value. Its
  // circular shape leaves more of the needle visible around the reading.
  value_panel = lv_obj_create(screen);
  lv_obj_set_size(value_panel, 84, 84);
  lv_obj_align(value_panel, LV_ALIGN_CENTER, 0, -2);
  lv_obj_set_style_bg_color(value_panel, lv_color_hex(0x101820), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(value_panel, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_width(value_panel, 0, LV_PART_MAIN);
  lv_obj_set_style_radius(value_panel, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_set_style_pad_all(value_panel, 0, LV_PART_MAIN);

  value_label = lv_label_create(screen);
  lv_obj_set_style_text_color(value_label, lv_color_hex(0xFFFFFF),
                              LV_PART_MAIN);
  lv_obj_set_style_text_font(value_label, &lv_font_montserrat_32,
                             LV_PART_MAIN);
  lv_obj_align(value_label, LV_ALIGN_CENTER, 0, -12);

  unit_label = lv_label_create(screen);
  lv_label_set_text(unit_label, "inH2O");
  lv_obj_set_style_text_color(unit_label, lv_color_hex(0xAEBBC6),
                              LV_PART_MAIN);
  lv_obj_set_style_text_font(unit_label, &lv_font_montserrat_14,
                             LV_PART_MAIN);
  lv_obj_align(unit_label, LV_ALIGN_CENTER, 0, 16);

  status_panel = lv_obj_create(screen);
  lv_obj_set_size(status_panel, 94, 25);
  lv_obj_align(status_panel, LV_ALIGN_CENTER, 0, 55);
  lv_obj_set_style_radius(status_panel, LV_RADIUS_CIRCLE, LV_PART_MAIN);
  lv_obj_set_style_border_width(status_panel, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(status_panel, 2, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(status_panel, LV_OPA_COVER, LV_PART_MAIN);

  status_label = lv_label_create(status_panel);
  lv_obj_set_style_text_color(status_label, lv_color_hex(0xFFFFFF),
                              LV_PART_MAIN);
  lv_obj_set_style_text_font(status_label, &lv_font_montserrat_14,
                             LV_PART_MAIN);
  lv_obj_center(status_label);

  lv_obj_t* caption = lv_label_create(screen);
  lv_label_set_text(caption, "SPIKE");
  lv_obj_set_style_text_color(caption, lv_color_hex(0x7F8C98), LV_PART_MAIN);
  lv_obj_set_style_text_font(caption, &lv_font_montserrat_14, LV_PART_MAIN);
  lv_obj_align(caption, LV_ALIGN_BOTTOM_MID, 0, -18);

  update_demo_value(current_demo_value);
}

void animate_synthetic_reading() {
  const uint32_t now = millis();
  if (now - last_sample_ms < kSampleIntervalMs) {
    return;
  }
  last_sample_ms = now;

  current_demo_value += demo_direction;
  if (current_demo_value >= kGaugeMaximum) {
    current_demo_value = kGaugeMaximum;
    demo_direction = -1;
  } else if (current_demo_value <= kGaugeMinimum) {
    current_demo_value = kGaugeMinimum;
    demo_direction = 1;
  }
  update_demo_value(current_demo_value);
}

}  // namespace

void setup() {
  Serial.begin(115200);
  Serial.println("LVGL 9 + LovyanGFX GC9A01 display spike");
  Serial.println("Synthetic gauge values; not sensor measurements.");

  display.init();
  display.setColorDepth(16);
  // LVGL stores RGB565 pixels little-endian; LovyanGFX must swap each word
  // before sending it MSB-first over SPI to the GC9A01.
  display.setSwapBytes(true);
  display.setRotation(0);
  display.fillScreen(TFT_BLACK);

  lv_init();
  lv_tick_set_cb(lvgl_tick_ms);
  lv_display = lv_display_create(kDisplayWidth, kDisplayHeight);
  lv_display_set_color_format(lv_display, LV_COLOR_FORMAT_RGB565);
  lv_display_set_flush_cb(lv_display, flush_to_gc9a01);
  lv_display_set_buffers(lv_display, draw_buffer, nullptr,
                         sizeof(draw_buffer), LV_DISPLAY_RENDER_MODE_PARTIAL);

  create_gauge_screen();
  Serial.printf("LVGL heap after screen creation: %u bytes available\n",
                static_cast<unsigned>(ESP.getFreeHeap()));
}

void loop() {
  animate_synthetic_reading();
  lv_timer_handler();
  delay(5);
}
