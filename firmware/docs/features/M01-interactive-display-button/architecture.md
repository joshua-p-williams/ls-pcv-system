# Architecture: Interactive Display and Button

## Purpose and scope

This document describes the M01 implementation design for the approved
[feature](feature.md), informed by the [research](research.md). It defines the responsibilities,
data flow, and test boundary for a synthetic display demonstration on the N16R8 bench setup. It
does not implement the feature or authorize work beyond the later reviewed task phases.

M01 uses the existing PlatformIO Arduino project, the N16R8, W01 wiring, the physical GC9A01
display, LVGL 9.2.2, LovyanGFX 1.2.35, and the project-authored BetterButton library. The finished-
gauge XIAO configuration, sensor/ADC acquisition, calibration, production thresholds, and persistent
settings remain outside this design.

## Design approach

Use a small hardware-independent application core with thin board and display adapters. This keeps
pressure classification, demo behavior, page state, trend history, and run extrema testable without
Arduino, LVGL, or attached hardware. Hardware-specific setup stays explicit for the N16R8.

The separation is a test and change boundary, not a requirement to create an interface for every
class. M01 has a real seam between the demo pressure generator and the application state, but it does
not need a generic service container, plugin system, or alternate-board abstraction yet. The future
sensor milestone can define its reading and validity contract when its calibration and fault
behavior are known; M01 should expose a plain pressure-update boundary without guessing that
contract now.

## Responsibilities

| Part | Responsibility | Depends on |
| --- | --- | --- |
| `DemoPressureSource` | Holds the current synthetic pressure and produces a bounded new value for a normal, warning, or danger request. Accept an injectable/seedable random source so tests are deterministic. | Standard C++ and demo band/range configuration |
| `PressureClassifier` | Classifies one signed pressure value using the four ordered M01 thresholds. This is the only place that implements threshold comparisons. | Standard C++ |
| `GaugeSession` | Holds the current pressure, derived status, active page, 30-entry rolling history, and current-run minimum/maximum. Accepts pressure updates, samples, and page changes through small explicit methods. | `PressureClassifier`, standard C++ |
| `M01Interaction` | Maps click to a normal demo request, double-click to a warning/danger request, and long press to the next page. Applies generated pressure to `GaugeSession`; it does not contain display code. | `DemoPressureSource`, `GaugeSession` |
| `BetterButtonInput` | Wraps BetterButton initialization and frequent polling; translates library event codes into a small M01 gesture enum. Each returned event is passed to the interaction handler once. | BetterButton, Arduino GPIO |
| `LvglGaugeDisplay` | Initializes the GC9A01 through LovyanGFX and LVGL, creates the three pages, and renders a read-only snapshot from `GaugeSession`. The gauge ribbon and status label use the classifier result supplied by the session. | LVGL, LovyanGFX, N16R8 display configuration |
| `main.cpp` | Composition root: creates the board-specific adapters and application objects, initializes them, and runs the nonblocking loop. | All concrete M01 parts |

Use simple data types for the pressure value, classified band, gesture, page, and display snapshot.
Keep pressure values signed and expressed in `inH2O`. The M01 demo source may generate tenths of
`inH2O` so the display can show fractional movement while the four configured thresholds remain
exact values. Invalid-sensor state is not part of this demo model; it belongs with the future
acquisition/calibration contract.

Keep the four demo thresholds and the visual minimum/maximum in one `DemoBandConfig` value shared by
the source, classifier, and display snapshot. The source uses it to bound generated values, the
classifier uses it to determine status, and the display uses it to place ribbon sections. This
prevents independent copies of the same thresholds from drifting apart.

## Data flow

1. `BetterButtonInput` polls BetterButton on every main-loop pass and reports a gesture when one is
   available. Polling must not wait for a gesture or delay LVGL service.
2. `M01Interaction` maps the gesture. A click asks `DemoPressureSource` for a value in the normal
   band; a double-click asks it to select a warning or danger band and choose a value within that
   band; a long press advances the page without changing the pressure.
3. When a generated value changes, the interaction handler passes it to `GaugeSession`. The
   session updates current pressure, classification, and current-run extrema.
4. A one-second elapsed-time check records the current pressure in the 30-entry rolling history.
   This timer runs whether the gauge, trend, or extrema page is visible. Button actions do not insert
   history points directly. Do not fabricate samples from before startup to prefill the trend.
5. `LvglGaugeDisplay` renders the current snapshot. The gauge, trend, and min/max pages are views of
   the same session, not separate sources of pressure state.
6. USB diagnostics report the synthetic source at startup and useful gesture/page/value events.
   Firmware startup and the main loop never wait for a serial monitor.

### M01 demo bands and gauge span

Use the revised provisional demo thresholds: lower danger `-12`, lower warning `-8`, upper warning
`-3`, and upper danger `0 inH2O`. Their increasing numeric order is required. The classifier
returns:

| Pressure | M01 display status |
| --- | --- |
| `-14` through values below `-12 inH2O` | Danger |
| `-12` through values below `-8 inH2O` | Warning |
| `-8` through `-3 inH2O`, inclusive | Normal |
| Above `-3` through `0 inH2O`, inclusive | Warning |
| Above `0 inH2O` | Danger |

Use `-14` to `+4.5 inH2O` as the initial M01 visible scale. These asymmetric endpoints are family-level
reference points from the service information for GM 16238399, which the purchased unmarked sensor
listing cross-references; they are not validated limits for the installed unit. The demo remains
synthetic. The family reference is a planning envelope, not a guaranteed full-scale range or
saturation curve. Zero is marked between vacuum and pressure, but need not be the visual midpoint.
The eventual live scale and numeric valid domain must come from calibration of the installed sensor;
software must not extrapolate beyond that calibration. Outside it, show `OUT OF RANGE` with direction
instead of a numeric pressure. Keep this distinct from pressure-band warning/danger and electrical
fault states. M01 itself only generates synthetic values inside the visible span; it does not
implement sensor out-of-range handling. Values beyond the family-reference visual span are not
generated or presented as pressure readings. Draw the colored ribbon sections from the same threshold configuration used by
`PressureClassifier`; do not hard-code independent visual ranges in LVGL. The normal band remains
green, warning amber/yellow, and danger red, with the numeric value/status treatment following the
accepted visual proof. If thresholds or the display span change, both classification and ribbon
must reflect the same configuration.

## Scheduling and UI behavior

Use elapsed-time comparisons rather than `delay()` or blocking waits. Each main-loop pass should:

1. Read the current millisecond counter.
2. Poll and dispatch available button events.
3. Record each due one-second trend sample from the current synthetic reading, advancing the
   rolling buffer once per elapsed sample interval without tying it to the active screen.
4. Refresh the display from the latest session snapshot when state changes; continue servicing
   LVGL's timer/handler often enough for animations and rendering.
5. Emit event diagnostics without blocking for USB serial availability.

The 30-entry ring buffer represents 30 one-second sample slots. Tests should exercise its initial
partial state, fill, rollover, and 30-second retention. Only samples collected since startup are
shown; do not imply a full prior 30-second history immediately after boot. A later trend drawing
choice can show elapsed-time labels and a signed pressure axis while preserving the stored sample
order.

Use LVGL's scale/section and label widgets for the gauge, its chart for the signed trend, and labels
for minimum/maximum values. Use the LVGL animation/timer facilities for visible transitions without
blocking the input loop. Keep the existing partial-buffer display approach as the starting point;
do not add double buffering, DMA tuning, or a fixed frame-rate subsystem without a demonstrated M01
need.

## Source layout

Keep hardware-independent rules in a PlatformIO private library so native tests can compile those
rules without compiling the Arduino entry point or the display stack. Keep the target composition
and adapters under `src/`:

```text
firmware/
  lib/
    gauge-core/
      include/gauge_core/    # stable M01 data types and declarations
      src/                   # classifier, demo source, session/history, interaction
    better-button/           # reviewed source copy, attribution, license, and provenance
  src/
    main.cpp                 # composition root and nonblocking loop
    board/n16r8/             # W01-derived pin and peripheral configuration
    input/                   # BetterButton-to-M01 gesture adapter
    display/                 # LVGL pages and LovyanGFX/GC9A01 setup
  test/
    test_gauge_core/         # Unity host tests for hardware-independent rules
```

The BetterButton copy is pinned and documented in its private library folder. Preserve its upstream
source and license; record any future adaptation and the reason beside that library. Keep
`lib_deps` versions for LVGL and LovyanGFX pinned to the versions proven by the existing spike.
Do not copy the experimental application into the production `src/`; reuse only the relevant
display initialization/configuration patterns and implement M01 behavior once in the default
firmware project.

## Testing design

Use PlatformIO's Unity framework with a native test environment for `gauge-core`. Configure the
native test run so it compiles the test files and referenced core library, not `src/main.cpp` or
the hardware/display adapters. Keep the ordinary `n16r8` environment as the build/upload target.
The native environment requires a host GCC compiler; document the supported Windows and Linux
setup and commands in the firmware guide when implementation is planned.

Host tests cover:

- Classification below, at, and above each threshold, including exact inclusive boundaries.
- Normal, warning, and danger generator bounds, deterministic seeded output, and generated values
  that remain inside the visible scale.
- One-second sampling cadence, initial partial history, 30-entry rollover, and persistence of the
  current value between button changes.
- Current-run minimum and maximum updates, including a single initial value and later extrema.
- Gesture-to-demo behavior and the three-page cycle, confirming page changes preserve current
  pressure, history, and extrema.

On-device N16R8 checks separately cover display initialization/readability, all three pages,
physical gesture recognition, prompt visible value changes, ribbon/status consistency, USB startup
diagnostics, and continued responsiveness during rendering. A passing host test or firmware build
does not verify the board wiring, physical button, display colors, power, or timing.

## Dependencies and failure boundaries

- The board pin configuration follows W01: display DC GPIO4, reset GPIO5, CS GPIO10, MOSI GPIO11,
  SCLK GPIO12, and the proposed N16R8 button input GPIO6 (W01 Q14). The button allocation is a
  documented proposal pending the normal physical pin and response checks. Do not reassign pins to
  suit simulation or display-library defaults.
- BetterButton's current timing macros are defaults, not requirements. Keep their values
  configurable or locally overridable only as supported by the reviewed implementation, and verify
  click/double-click/long-press response on the physical button. An unavailable or failed button
  should not stop display refresh or serial startup diagnostics.
- A display initialization failure should be reported over USB when available without indefinite
  waiting. M01 has no sensor, ADC, or measurement-fault behavior to implement.
- No XIAO target, vehicle power configuration, calibration persistence, production alarm behavior,
  or real pressure source is introduced in M01.

## Decisions deferred to implementation planning

The architecture intentionally leaves visual details such as tick spacing, trend grid/labels,
page-indicator styling, and exact text layout to implementation tasks and physical review. The
exact BetterButton upstream revision and compilation result must be recorded during integration.
If source inspection reveals a concrete compatibility issue, update the design with the evidence
before choosing an alternate input implementation.
