# Feature: Interactive Display and Button

## What is this feature?

M01 adds the first interactive, visible firmware behavior to the crankcase pressure gauge. On the
N16R8 development board, the GC9A01 display presents a signed synthetic pressure reading, and the
multifunction button changes the generated reading or moves between gauge pages. This exercises
the display and input hardware together before sensor acquisition and engine measurements are
introduced.

The display proof showed that the selected GC9A01 can render the gauge concept on the N16R8. The
tested LVGL/LovyanGFX stack is recorded in [ADR 0001](../../decisions/0001-m01-graphics-stack.md);
the proof remains an isolated experiment while M01 integrates the reviewed behavior into the
firmware project.

## Why it matters

The display and button are the gauge's primary user interface. Exercising them with controlled,
synthetic values provides immediate feedback on the physical hardware and establishes the basic
input-to-state-to-display path before analog acquisition, calibration, and pressure interpretation
increase the number of possible failure causes.

The demonstration also gives the repository a concrete instructional example of how the gauge
represents vacuum, pressure, status bands, recent history, and captured extrema. Synthetic values
make the visuals testable without implying that any reading or threshold has been measured on the
engine.

## What "done" looks like

- The default PlatformIO firmware project builds for the N16R8 configuration and initializes the
  GC9A01 using the reviewed display stack recorded in ADR 0001.
- The physical multifunction button is read on the N16R8 bench setup using BetterButton, authored
  for use in this project. Its source may be copied or adapted into the firmware repository as
  needed. Research should verify the relevant API and ESP32 compatibility and identify the source
  revision used; permission to use the library is not an open question.
- A synthetic pressure source supplies signed values in `inH2O`. It is clearly named and documented
  as generated data, and startup USB diagnostics identify that source. The screen does not need a
  synthetic/demo label.
- The gauge page displays the current signed value and unit, distinguishes vacuum from pressure
  around a marked zero reference, shows a moving indicator, and presents a status that is visually
  distinct from pressure sign.
- Four demo-only thresholds classify values into lower danger, lower warning, normal, upper
  warning, and upper danger bands. Use `-12`, `-8`, `-3`, and `0 inH2O` so the synthetic display
  exercises its lower danger band within the cited family-level `-14 to +4.5 inH2O` reference
  envelope. These are visual test values, not validated limits for the engine, installed sensor,
  or production alarm settings. See the [sensor reference and calibration limits](../../../../hardware/components/fuel-tank-pressure-sensor/README.md#published-reference-points-for-the-advertised-gm-family).
- The screen shows the three agreed pages: the main gauge, a rolling 30-second pressure trend, and
  minimum/maximum values accumulated during the current run. The trend and extrema use the same
  synthetic reading source as the main gauge. A nonblocking one-second timer records the current
  synthetic value in a 30-entry rolling buffer; button gestures change the current value but do not
  insert samples directly. The buffer advances independently of the selected page.
- The temporary M01 button mapping works as follows:
  - A click generates a new value in the normal band.
  - A double-click generates a new value in one of the warning or danger bands.
  - A long press cycles gauge → trend → min/max → gauge.
  - Reading changes are reflected on the currently selected page; changing pages does not reset
    the generated reading, trend window, or run extrema.
- Gauge movement is animated or otherwise visibly transitions between generated values without
  blocking button processing or serial diagnostics. A fixed frame rate or refresh optimization is
  not required unless physical testing shows a specific usability problem.
- The active page is identifiable, and the display remains readable with the synthetic pressure
  and status colors. Normal is dark in the value hub; warning and danger use the reviewed amber
  and red treatments.
- USB diagnostics identify the demo source and report useful button/page events without waiting
  indefinitely for a serial monitor.
- Automated software tests exercise signed pressure classification at and around every threshold,
  generated-value bounds/reproducibility, the 30-second history behavior, and run min/max tracking.
  On-device checks separately verify display initialization, all page transitions, button gestures,
  and visible updates.
- Implementation follows the [firmware coding standards](../../coding-standards.md): document
  public contracts and non-obvious behavior, keep application rules separate from hardware and UI
  details, and use focused abstractions rather than speculative framework layers.
- M01's gesture mapping and synthetic source are explicitly temporary. They do not change the
  planned production button behavior, implement zeroing or reset actions, or provide real engine
  monitoring.

## What this feature is deliberately not

M01 does not read the FTP sensor or ADS1115, validate the RC filter, calibrate pressure, establish
engine-specific warning limits, or interpret the displayed values as engine measurements. It does
not implement the finished-gauge XIAO configuration, persistent settings, a settings menu, vehicle
behavior, engine-data integration, or logging. It does not require a visual UI designer, DMA,
double buffering, or display refresh optimization as part of the initial implementation.

The existing LVGL/GC9A01 spike remains a reference and proof artifact. It is not a second firmware
application and should not become a duplicate implementation of the M01 behavior.
