#ifndef LV_CONF_H
#define LV_CONF_H

// This is an exploratory, deliberately small LVGL configuration for the
// GC9A01 compatibility test. It is not the production firmware configuration.
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0
#define LV_MEM_SIZE (48U * 1024U)
#define LV_DRAW_SW_ASM LV_DRAW_SW_ASM_NONE
#define LV_USE_SCALE 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_32 1

#endif  // LV_CONF_H
