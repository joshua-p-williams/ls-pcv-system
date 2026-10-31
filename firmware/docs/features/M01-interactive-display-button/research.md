# Research: Interactive Display and Button

## Purpose and scope

This document records the implementation-relevant findings for M01. It supports the feature
definition and upcoming architecture discussion; it does not define the final module structure or
authorize implementation. M01 is a synthetic demonstration on the N16R8 bench configuration,
not a crankcase measurement or a validated alarm system.

## Project constraints confirmed

- The default firmware project is PlatformIO with Arduino, pinned to `espressif32@8.0.0`, and targets
  the project-local `esp32-s3-n16r8` board profile. Its current program is the M00 heartbeat.
- W01 is the connectivity authority. The current display assignments are GPIO4 for DC, GPIO5 for
  reset, GPIO10 for chip select, GPIO11 for MOSI, and GPIO12 for clock. The multifunction button
  connects its assigned GPIO to ground and uses the controller's input pull-up. Consult W01 and its
  generated schedule for the complete terminal map; this research does not duplicate it.
- The N16R8 and physical 240 x 240 GC9A01 have already demonstrated the selected graphics stack.
  The retained [LVGL/GC9A01 spike](../../../experiments/lvgl-gc9a01/README.md) records the physical
  visual review, pin use, draw-buffer approach, demo-only thresholds, and limits of that proof.
- [ADR 0001](../../decisions/0001-m01-graphics-stack.md) selects LVGL 9.2.2 with LovyanGFX 1.2.35
  as M01's starting stack. The selection is limited to M01 on the current bench display; it does not
  prove XIAO performance or settle the finished gauge's stack.

## Graphics and display findings

LVGL 9.2 provides a scale widget with ticks, major labels, styled sections, and custom labels. This
fits the signed, zero-centered dial and its distinct normal, warning, and danger regions. The scale
does not decide pressure classification; application logic should supply the already-classified
status so threshold comparisons have one source of truth. LVGL 9.2's chart widget supports line
charts, configurable point counts, and shift or circular update modes. These are suitable drawing
primitives for M01's gauge and trend page, while page layout and time-window behavior remain
application decisions. See the [LVGL 9.2 scale documentation](https://lvgl.io/docs/9.2/widgets/scale)
and [chart documentation](https://lvgl.io/docs/9.2/widgets/chart).

LovyanGFX documents Arduino ESP32 support and SPI LCD support. More importantly for this specific
project, the exact selected versions have already rendered the project's physical GC9A01 from the
N16R8. That evidence is stronger for M01 compatibility than a general library feature list. The
proof still does not establish a refresh-rate target, XIAO support for the eventual display setup,
or behavior under concurrent ADC acquisition. Those are outside M01 or require later tests. See
the [LovyanGFX project](https://github.com/lovyan03/LovyanGFX) and the local proof record.

No visual UI designer is needed to proceed. LVGL widgets and hand-authored layout are adequate for
the three small M01 pages. A designer can be reconsidered if screen authoring becomes difficult to
maintain, as already recorded in the brainstorm and ADR.

## BetterButton findings

The public BetterButton README and header describe an Arduino C++ class constructed with a GPIO
number, initialized with `begin()`, and polled through `input()`. `input()` returns gesture event
codes for click, double click, and long press. The README says the button connects the GPIO to
ground, `begin()` enables the internal pull-up, and `input()` should be called frequently in the
main loop; an emitted event must be handled or saved because the library treats it as consumed. The
header currently defines 60 ms debounce, 500 ms double-click monitoring, and 2000 ms long-press
constants. These are current library defaults, not M01 requirements. See the
[BetterButton README](https://github.com/joshua-p-williams/BetterButton) and
[public header](https://github.com/joshua-p-williams/BetterButton/blob/master/BetterButton.h).

The library is authored by the project contributor and permission to use, copy, or adapt it is
settled. The exact source revision was inspected and copied under its MIT license; provenance and
behavior are recorded in the [private library README](../../../lib/better-button/README.md).
The copied library compiled successfully in the N16R8 PlatformIO environment. Its physical gesture
timing remains to be checked under the integrated loop and display load. Preserve the ability to
tune timing rather than treating upstream defaults as fixed behavior. No separate event queue is
warranted unless the eventual loop design can defer handling an emitted event.

## Synthetic readings and display bands

The initial feature and brainstorm drafts proposed lower danger `-16`, lower warning `-8`, upper
warning `-3`, and upper danger `0 inH2O`. The `-16` value is outside the cited family reference
envelope and has been superseded for M01 by `-12`; the other values remain provisional display
boundaries. The `-8` to `-3 inH2O` interval matches the broad provisional tuning target. None of
these values is a validated limit for this engine, a sensor calibration value, or a production alarm
setting. The component documentation and project guidance require an installed-chain calibration
and measured operating baseline before real thresholds are selected.

The double-click generator should select a warning or danger band and then generate a value within
that band's bounds. It should not infer status from pressure sign: all four boundaries are signed
positions on one number line, and a negative value can be warning or danger. Make the random source
injectable or seedable so boundary and range tests are repeatable; normal demonstrations may use a
varying seed. Keep the synthetic source named and reported as synthetic in code and startup USB
diagnostics, without adding a demo label to the screen.

The spike used a `-20` to `+10 inH2O` display span, an exploratory visual choice that exceeds the
family-level pressure reference on both sides. The 2002 Impala service description for GM 16238399
gives approximately `-14 inH2O` at `4.5 V`, atmospheric pressure at `1.3–1.7 V`, and `+4.5 inH2O`
at `0.5 ± 0.2 V`. The source describes reference points, not guaranteed full-scale accuracy or
clipping behavior. The purchased unmarked aftermarket sensor only claims cross-compatibility with
this part family, so actual usable bounds and transfer behavior require calibration. See the [sensor
record](../../../../hardware/components/fuel-tank-pressure-sensor/README.md#published-reference-points-for-the-advertised-gm-family)
and [2002 Impala service description](https://charm.li/Chevrolet/2002/Impala%20V6-3.4L%20VIN%20E/Repair%20and%20Diagnosis/Powertrain%20Management/Computers%20and%20Control%20Systems/Fuel%20Tank%20Pressure%20Sensor/Description%20and%20Operation/).

Accordingly, M01 uses a synthetic visual span of `-14` to `+4.5 inH2O` and adjusts its lower-danger
demo point to `-12 inH2O`. This keeps the demonstration inside the sourced family reference envelope;
`-12` remains a UI test value with no claimed engine-health meaning. The user-provided provisional
hot-idle tuning target (`-3 to -5 inH2O`, potentially extending to `-8`) is separate from warning
and critical limits. No lower engine-critical threshold is established. Positive pressure may be
treated conservatively as a provisional warning/danger policy after baseline testing, but it is not
an OEM failure threshold. Real firmware must use the installed sensor's calibrated valid domain and
show out-of-range instead of extrapolating; actual saturation behavior is unknown.

## Trend sampling decision

LVGL can draw and rotate through chart points, but a chart widget does not define what “30 seconds”
means. The agreed M01 model is a timer-driven history: every one second, the firmware records the
current synthetic pressure into a 30-entry rolling buffer. A button gesture changes the current
value; it does not insert a history point immediately. The next timer tick records the new value,
which remains represented at subsequent ticks until another gesture changes it. The buffer
therefore represents 30 consecutive one-second sample slots and advances independently of page
navigation.

Use a nonblocking elapsed-time check for the one-second cadence. Make the clock injectable in
software tests so buffer rollover and sample timing are reproducible without real delays. The
architecture should make clear that the 30 entries are one-second samples, not an event log; the
time labels should match that representation. This is a demonstration sampling cadence, not a
future sensor acquisition-rate decision.

## Test approach supported by the toolchain

PlatformIO Unit Testing supports both native host tests and tests on embedded targets. Its
documentation describes using host tests for hardware-independent logic and embedded tests for
target-specific behavior. PlatformIO's Unity integration supports both native and embedded tests;
the Native platform requires a host compiler installed separately. This supports a small M01 split:

- Host tests for pressure-band boundaries, deterministic generated-value bounds, page-state
  transitions, rolling-window retention, and run min/max behavior. Keep these rules independent of
  LVGL and Arduino hardware APIs so they can execute without the board.
- Manual N16R8 checks for display initialization, readable page layouts, gesture recognition,
  visible value changes, and continued USB diagnostics. These checks validate physical behavior;
  passing host tests does not establish display or button wiring.

**Recommendation:** use PlatformIO's Unity framework in a native host-test environment for M01's
hardware-independent application rules. Unity supports both native and embedded targets, is small,
and avoids bringing in a mocking framework when the planned rules can be tested as plain C++.
PlatformIO also supports richer host-only frameworks, but their additional features are not needed
for this milestone. The Native environment requires a host GCC toolchain: on Windows, the PlatformIO
guide describes installing MSYS2 GCC; on Linux, it uses the system GCC/build tools. This is a small
additional Windows setup step and should be documented in the M01 test instructions. Keep hardware
checks separate and manual on the N16R8. See [PlatformIO Unit Testing](https://docs.platformio.org/en/latest/advanced/unit-testing/index.html),
[Unity support](https://docs.platformio.org/en/latest/advanced/unit-testing/frameworks/unity.html),
and [the Native platform requirements](https://docs.platformio.org/en/latest/platforms/native.html).

## Research conclusions and remaining design inputs

The existing evidence is sufficient to proceed to M01 architecture: retain the selected LVGL 9.2.2
and LovyanGFX 1.2.35 stack for this milestone, use BetterButton as the planned input library subject
to compile/bring-up verification, and test the application rules independently of the display.
Neither a broad library re-evaluation nor a visual designer is needed.

The architecture discussion should resolve or explicitly choose defaults for:

1. **M01 gauge span:** resolved as the family-reference span `-14` to `+4.5 inH2O` for synthetic
   UI work only; live numeric bounds remain calibration-dependent.
2. **Host-test configuration:** apply the Unity/native recommendation in the architecture and
   document the Windows GCC prerequisite alongside the Linux host-toolchain requirement.
3. **BetterButton integration:** select a source layout, pin the upstream revision, compile the
   library, and verify gesture timing on the physical button. The user has already approved its use;
   this is an implementation/provenance check rather than a permission question.

The initial lower-danger value `-16` and exploratory `-20 to +10` span are superseded by the sourced
family reference envelope. M01 uses `-12, -8, -3, 0 inH2O` strictly to exercise the display states;
they do not define production operating limits.
