# LVGL and GC9A01 exploratory spike

This isolated PlatformIO project is a retained learning artifact from the M01
graphics-library proof. On the N16R8 bench setup, LVGL 9.2.2 with LovyanGFX
1.2.35 successfully drove the selected 240 x 240 GC9A01 and rendered a signed,
zero-centered gauge with colored bands, a status indicator, and a moving
needle. The display, colors, text, and needle alignment were visually reviewed
on the physical display. This demonstrates that the combination is viable for
further M01 work; it does not make the libraries a final production-stack
decision.

The spike uses the W01 N16R8 pin allocation and generated values only. It does
not read the FTP sensor, ADS1115, or button; implement the three planned pages;
or represent production pressure limits. It is not production firmware. The
default `firmware/` PlatformIO project remains independent and unchanged.

The values `-16`, `-8`, `-3`, and `0 inH2O` are the provisional display-spike
bands recorded in the M01 brainstorm. They are only visual test values, not
validated engine limits. The `SPIKE` caption and serial startup
message deliberately identify this as an experiment.

## Build, upload, and observe

Open this directory as a PlatformIO project, or run the commands from it:

```text
pio run
pio run -t upload
pio device monitor -b 115200
```

If automatic upload does not connect, use the same BOOT/RESET download-mode
procedure documented in the [firmware setup guide](../../docs/development-environment.md).
The display is wired according to the display/button phase of [W01](../../../hardware/wiring/bench/README.md#phase-2-display-and-button):
GPIO4 DC, GPIO5 reset, GPIO10 chip select, GPIO11 MOSI, and GPIO12 clock.
The display and controller must share ground; VCC is the W01-selected 3.3 V.

This spike uses LVGL 9.2.2 and LovyanGFX 1.2.35, with a single 32-row partial
RGB565 buffer. The driver directly connects LVGL's flush callback to LovyanGFX
so the test also exercises the integration layer. No calibration, input
handling, persistence, business logic, or application architecture is
included. The experiment can be removed without changing the default
`firmware/` heartbeat project.

## Proof outcome and limits

- Physical display operation, color order, orientation, value readability,
  status colors, and needle alignment were reviewed and accepted as a successful
  visual proof.
- The moving display can show repaint/motion artifacts. Refresh optimization
  remains optional; changes such as DMA or double buffering should be justified
  by a specific visible problem before adding their implementation complexity.
- Button input, multi-page layout, trend history, min/max behavior, pressure
  classification tests, and production architecture were not part of this
  proof. Those belong in the reviewed M01 feature documents and implementation.

The visual findings above are based on the reported assembled-board review. No
sensor behavior, calibration, or real engine limits were tested.
