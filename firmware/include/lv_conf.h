#ifndef LV_CONF_H
#define LV_CONF_H

// LVGL is the widget/layout/animation library; LovyanGFX handles the panel bus.
// Keep its build-time options in this project file so the pinned LVGL library
// uses the same color format and features as the GC9A01 display adapter.
#define LV_COLOR_DEPTH 16
// RGB565 stores each pixel in 16 bits (5 red, 6 green, 5 blue).
#define LV_COLOR_16_SWAP 0
// LVGL's internal allocation pool for widgets, styles, and draw bookkeeping.
#define LV_MEM_SIZE (48U * 1024U)
// Let LVGL use portable software drawing rather than architecture-specific asm.
#define LV_DRAW_SW_ASM LV_DRAW_SW_ASM_NONE
// The gauge uses the LVGL scale widget for the colored signed dial and needle.
#define LV_USE_SCALE 1
// Include only font sizes used by M01's 240 x 240 pages to control flash use.
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_10 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_32 1

#endif  // LV_CONF_H
