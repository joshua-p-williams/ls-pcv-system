# ADR 0001: LVGL and LovyanGFX for M01

**Status:** Accepted for M01

## Context

M01 needs a circular, signed-pressure gauge and three display pages on the 240 x 240 GC9A01 SPI
TFT. The display stack should support the gauge, status colors, text, and future trend/min-max
views without requiring hand-built graphics primitives for every element.

An isolated proof ran on the N16R8 bench board with the physical display. LVGL 9.2.2 rendered the
gauge, colored bands, status, and moving needle through LovyanGFX 1.2.35. The proof and its limits
are recorded in the [exploratory spike](../../experiments/lvgl-gc9a01/README.md).

## Decision

Use LVGL 9.2.2 with LovyanGFX 1.2.35 as the starting graphics stack for M01. Pin the dependencies
in the M01 firmware configuration. If research finds a material compatibility, maintenance, or
resource issue, document the evidence and review the proposed change before replacing the stack.

This decision applies to M01 on the N16R8 and its current GC9A01 display. It does not establish the
permanent graphics stack for the finished gauge or prove XIAO performance, display readability in
a vehicle, or sensor/measurement behavior.

## Alternatives considered

- **LovyanGFX alone:** It can draw directly to the GC9A01, but hand-authoring the planned gauge and
  multiple pages would require maintaining more UI primitives and state-specific drawing code.
- **TFT_eSPI with hand-drawn screens:** It is a viable lightweight alternative for a small custom
  interface, but would require a separate integration proof and more custom gauge/layout code.
- **A visual UI designer:** It may help if hand-authored LVGL screens become difficult to maintain,
  but is not needed for the current scope and is not a mandatory tool.

## Consequences

- M01 can use LVGL's scale, labels, styles, and screen objects while LovyanGFX handles the panel and
  SPI transfer.
- The firmware must configure LVGL, its draw buffer, the GC9A01 panel, and the LVGL flush path.
  This brings additional dependency and memory costs compared with drawing directly through a
  display driver.
- The proof showed acceptable visual results, but it did not implement the button, all three pages,
  or production firmware boundaries. Refresh optimization is not required unless testing identifies
  a specific usability problem.
- Keep the display driver behind a presentation boundary so pressure classification and synthetic
  source behavior do not depend on LVGL widget APIs. The M01 architecture defines the concrete
  boundary.
