#pragma once

#include <LovyanGFX.hpp>

// Minimal LovyanGFX panel/bus setup for the W01 N16R8 bench GPIO allocation.
// These are the documented proposed W01 pins; confirm the assembled board
// matches W01 before powering the display.
class LGFX : public lgfx::LGFX_Device {
 public:
  LGFX() {
    auto bus_config = bus_.config();
    bus_config.spi_host = SPI2_HOST;
    bus_config.spi_mode = 0;
    bus_config.freq_write = 20000000;
    bus_config.freq_read = 10000000;
    bus_config.spi_3wire = false;
    bus_config.use_lock = true;
    bus_config.dma_channel = SPI_DMA_CH_AUTO;
    bus_config.pin_sclk = 12;  // W01 DISP_CLK / GPIO12
    bus_config.pin_mosi = 11;  // W01 DISP_MOSI / GPIO11
    bus_config.pin_miso = -1;  // The selected display header exposes no MISO.
    bus_config.pin_dc = 4;     // W01 DISP_DC / GPIO4
    bus_.config(bus_config);
    panel_.setBus(&bus_);

    auto panel_config = panel_.config();
    panel_config.pin_cs = 10;  // W01 DISP_CS / GPIO10
    panel_config.pin_rst = 5;  // W01 DISP_RST / GPIO5
    panel_config.pin_busy = -1;
    panel_config.panel_width = 240;
    panel_config.panel_height = 240;
    panel_config.offset_x = 0;
    panel_config.offset_y = 0;
    panel_config.offset_rotation = 0;
    panel_config.readable = false;
    panel_config.invert = true;
    panel_config.rgb_order = false;
    panel_config.dlen_16bit = false;
    panel_config.bus_shared = false;
    panel_.config(panel_config);

    setPanel(&panel_);
  }

 private:
  lgfx::Bus_SPI bus_;
  lgfx::Panel_GC9A01 panel_;
};
