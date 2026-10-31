#ifndef LV_CONF_H
#define LV_CONF_H

// M01 starting LVGL configuration, based on the physical GC9A01 compatibility
// proof. Tune only when the feature implementation demonstrates a need.
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0
#define LV_MEM_SIZE (48U * 1024U)
#define LV_DRAW_SW_ASM LV_DRAW_SW_ASM_NONE
#define LV_USE_SCALE 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_10 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_32 1

#endif  // LV_CONF_H
