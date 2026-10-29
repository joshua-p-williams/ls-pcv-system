# M01 — Interactive Display and Button Brainstorm

This note captures the current discussion for M01. It is input to the feature definition, not a
feature specification, implementation plan, or authorization to implement. Review and revise it
before drafting `feature.md`.

## Desired outcome

Use the N16R8 bench setup to bring the GC9A01 display and multifunction button to life in a
small, visible firmware demonstration. The display should introduce the eventual crankcase gauge
concept, while generated pressure values make the screens useful before the sensor and ADC are
implemented in firmware.

## Agreed direction

- M01 runs on the N16R8 bench configuration. It initializes the GC9A01 and reads the documented
  multifunction button. The finished-gauge XIAO configuration remains a later validation target.
- Demonstrate three screens: a primary bidirectional gauge, a 30-second pressure trend, and a
  minimum/maximum view. Use synthetic values only; identify their source in code, comments,
  documentation, and startup diagnostics rather than with an on-screen marker.
- The main screen centers a signed pressure value in `inH2O`. Negative values represent vacuum;
  positive values represent pressure. Use zero as the reference separating the two directions.
- The gauge face should have a sweep that communicates the current value and a status indicator
  that can be understood separately from the pressure sign. Use green, yellow, and red visual
  bands like a tachometer; the displayed value or its nearby panel may also reflect the active
  band. The supplied concept sketch is a
  visual reference for its round face, dark palette, large reading, and distinct vacuum/pressure
  colors. Refine it with a visible zero reference, labels for both directions, and a moving
  needle/marker or equivalent indication of the current value.
- A long press cycles through the three pages in order—gauge, trend, min/max, then back to gauge—
  with a small page indicator. This is a temporary M01 interaction only; it does not establish
  the production meaning of long press or supersede the later gauge behavior already recorded in
  `firmware/README.md`.
- Do not add an on-screen synthetic/demo marker. The source should be unmistakably named and
  documented in code, and startup diagnostics should report that the M01 demo pressure source is
  active. This keeps the display clean while making the implementation and its operating mode
  clear to contributors.
- Keep display, page state, and pressure source sufficiently separate that synthetic values can
  later be replaced by acquired and calibrated values without tying measurement logic to the
  GC9A01 rendering code. Keep this boundary small and proportional to the demonstration.
- A visual UI designer is optional. Record it as a tool to revisit only if hand-authored screens
  become difficult to design or maintain.

## Demo pressure bands and gestures

Define the M01 demo's visual bands with four independently configurable signed-pressure
thresholds: lower danger, lower warning, upper warning, and upper danger. These names describe
ordered positions on the signed number line, not negative-pressure and positive-pressure alarm
categories. All four thresholds may be negative. In increasing numeric order, with `p` as signed
pressure:

```text
p < lowerDanger                   red
lowerDanger <= p < lowerWarning   yellow
lowerWarning <= p <= upperWarning green
upperWarning < p <= upperDanger   yellow
p > upperDanger                   red
```

More precisely, the ordered boundaries are `lowerDanger < lowerWarning < upperWarning <
upperDanger`. The interval between the two warning thresholds is the green target band; the two
warning intervals transition to the danger regions outside the outer thresholds. Implement the
classification once and make drawing code consume the resulting status rather than reimplementing
threshold comparisons.

For a concrete M01 display demonstration, use these initial **demo-only** band constants, all in
inH2O:

| Boundary | Initial demo value | Basis and limitation |
| --- | ---: | --- |
| Lower danger | -16 | Comparison point identified for different GM engine families; not an LS1/408 limit |
| Lower warning | -8 | Edge of the project's provisional hot-idle/light-cruise target band |
| Upper warning | -3 | Other edge of the project's provisional hot-idle/light-cruise target band |
| Upper danger | 0 | Conservative comparison point: GM diagnostic material treats positive pressure as a concern in specific applications |

The ordered demo values are `-16 < -8 < -3 < 0`. They make the current demo green band align
with the provisional idle/light-cruise target and provide warning/danger regions to exercise the
visuals. Any positive synthetic reading falls in the upper danger band under these initial demo
values. These constants are not validated limits for the modified 408, are not suitable for
classifying real sensor data, and may be revised during M01 if they make the visual demonstration
unclear. Keep them separate from future live thresholds, which require repeatable idle, cruise,
and WOT baselines and review of operating-condition effects.

For the temporary M01 button mapping:

- A click chooses a generated reading within the green demo band.
- A double-click chooses a warning or danger band and generates a reading within it. Sampling is
  not tied to the reading's sign: a warning or danger band may contain values below or above zero.
- A long press advances through the gauge, trend, and min/max screens.

Use a bounded random source for the running demo. Make its seed/injection controllable so software
tests can reproduce values at and around every threshold. The UI does not need a `SIM`/`DEMO`
label, but USB startup diagnostics, source names, comments, and documentation must clearly
identify generated readings. These gesture actions are temporary and do not define future alarm,
zeroing, peak-reset, or settings behavior.

The main gauge centers the signed value, with zero marked on the scale and vacuum/pressure
directions labeled. Pressure sign and band status are separate: a value may be negative while in
the upper warning or danger region. The 30-second page shows a rolling trend of generated values.
Recommended min/max behavior for feature review is to accumulate over the current demo session
(since boot); this is simple to explain and distinct from a rolling trend. Keep the reset gesture
and production peak semantics separate for later design.

## Boundaries

M01 does not read the FTP sensor or ADS1115, establish sensor calibration, validate the RC filter,
set production engine warning thresholds, or implement vehicle behavior. M01's four code
constants are provisional demo display bands only. Production threshold values and operating
conditions require baseline readings at idle, cruise, and WOT and review of their interpretation.

The initial screen colors and synthetic readings exercise the demo bands; they are not validated
limits.
The gauge's eventual pressure range, tick spacing, warning colors, and status rules remain open
until supported by measurement and design review.

The long-press page cycle is deliberately temporary. Future button actions—including atmospheric
zero confirmation, min/max reset, menus, and persistent settings—remain governed by the later
milestone/design work and are not implemented or changed by this demo mapping.

## Research notes

Research reviewed on 2026-10-25 and 2026-10-28 informs candidates, not a final
graphics-stack selection:

- **BetterButton:** The [project README](https://github.com/joshua-p-williams/BetterButton) and
  [header](https://github.com/joshua-p-williams/BetterButton/blob/master/BetterButton.h) describe
  click, double-click, and long-press events, a button wired from a GPIO to ground, and polling
  through `input()`. The header currently defines 60 ms debounce, 500 ms double-click monitoring,
  and 2000 ms long-press constants. These are library defaults, not approved M01 timing
  requirements. The implementation and compatibility with this firmware still need review; pin
  an exact revision and preserve its license terms if adopted.
- **LVGL 9:** The [LVGL 9 scale widget](https://lvgl.io/docs/open/9.2/widgets/scale) supports
  circular scales, ticks, labels, and styled sections. LVGL also provides arcs and labels useful
  for the planned pages. Do not copy LVGL 8 `lv_meter` examples as if they were the LVGL 9 API;
  verify how a separate needle or marker should be rendered in the selected version.
- **Crankcase-pressure references:** A [GM diagnostic article](https://www.centerlearning.com/tmswebtree/techlink/images/issues/arcv_pdf/3_01_e.pdf)
  says specifications vary by engine family and recommends comparison with a known-good engine.
  GM documents for other families show materially different hot-idle ranges: [PIP5197B for
  selected 1.4 L models](https://static.nhtsa.gov/odi/tsbs/2015/MC-10114424-9999.pdf) reports
  -11 to -18 inH2O and flags positive pressure or values below -30 inH2O for further diagnosis;
  [GM TechLink for selected 1.4 L / 1.5 L models](https://gm-techlink.com/wp-content/uploads/2017/04/GM_TechLink_04_Mid-February_2017.pdf)
  reports -1 to -5 inH2O at hot idle and flags values below -16 inH2O. These are not
  specifications for the 2002 LS1 or this modified 408 and custom PCV system; they demonstrate
  why those values should not be copied as its validated limits.
- **LovyanGFX:** Its [project documentation](https://github.com/lovyan03/LovyanGFX) lists Arduino
  ESP32 support and the GC9A01 display controller, making it a candidate low-level display driver
  if LVGL is selected.
- **TFT_eSPI:** The [project](https://github.com/Bodmer/TFT_eSPI) includes a GC9A01 setup example
  and provides direct drawing primitives. It is a candidate if the UI remains simple enough to
  draw by hand without a full GUI framework; its setup and pin configuration need to match this
  project's PlatformIO layout.
- **EEZ Studio:** The [project overview](https://github.com/eez-open/studio) reports LVGL 8.x and
  9.x support. Its editable project can be considered if screen authoring becomes cumbersome.
  The editor is GPLv3; the project documentation says generated output can use a selected license,
  so generated files and template content still need review before adoption.
- **SquareLine Studio:** The [product and licensing pages](https://squareline.io/pricing/licenses)
  describe a visual editor for LVGL and personal-use licensing. It remains an optional candidate;
  review current terms and generated-code workflow before using it in this public repository.
- Community examples can help with controller/display initialization and gauge composition, but
  they are references rather than drop-in firmware. The
  [ESP32-S3/GC9A01/LVGL example](https://github.com/UsefulElectronics/esp32s3-gc9a01-lvgl) uses
  ESP-IDF, while this project currently uses Arduino through PlatformIO. The supplied
  automotive-gauge examples also vary in MCU, LVGL version, framework, and license clarity.
  In particular, review both repository metadata and README license statements before reusing
  code from the cited Custom-Car-Gauge project; no community source has been selected or copied.

The current preference is to evaluate a minimal working display slice before committing to a
GUI framework: verify GC9A01 initialization, draw a signed zero-centered dial and update a marker,
then decide whether the multi-page interface benefits enough from LVGL to justify its integration
and configuration. This is a research direction, not yet an adopted library decision.

## Recommended answers for feature review

- **Numeric demo thresholds:** start M01 with the four demo-only candidates listed above. Preserve
  the source and limitation beside each constant in code. They are for exercising the display,
  not production defaults; live thresholds require engine-specific baseline evidence.
- **Min/max page:** accumulate minimum and maximum since the demo starts. A later reset action can
  be specified with production peak capture rather than added to this temporary gesture map.
- **Hardware assumptions:** use the existing W01/P05 connections and accepted component/listing
  assumptions. Perform ordinary unpowered continuity, polarity/rail, display-initialization, and
  button-response checks during bring-up. Require additional investigation only if a concrete
  mismatch or failed check changes the design.
- **Graphics stack:** include a focused LVGL 9 compatibility spike with the GC9A01: initialize the
  panel, render a zero-centered signed scale with colored sections, and move an indicator. Keep
  LovyanGFX as the likely display-driver candidate and compare hand-drawn rendering only as needed
  to understand integration cost. Do not make a visual designer mandatory or turn the spike into
  a broad library survey.

## Code structure, comments, decisions, and tests

The demo and future sensor path should share small, explicit contracts where this avoids replacing
the display/application later. A demo pressure source can provide signed samples now; an ADC-backed
source can implement the same contract in a later feature. A pressure-band classifier should own
the threshold comparison, while the gauge renderer consumes a value and classified band. Avoid
creating generic service frameworks or interfaces without a near-term second implementation.

M01 feature research should define a proportional testing approach: repeatable software tests for
signed value handling, all four threshold boundaries, generated-value ranges, trend history, and
min/max tracking; separate on-device checks for display initialization, visual page changes, and
physical button recognition. Random demo values must be deterministic under tests.

Use code comments to explain intent, constraints, and why demo behavior differs from future live
behavior. Mark temporary paths and constants clearly (for example, with `Demo` names and comments
that identify their replacement point); do not add comments that merely narrate obvious syntax.
Record lasting design choices in concise ADRs where a decision has meaningful alternatives and
future consequences, such as a graphics-stack selection or a shared pressure-source contract.
Comment conventions and test commands belong in a small firmware development standard, not in an
ADR for every routine practice.

## Relationship to later work

This demonstration is a user-visible foundation for later measurement features. Future milestones
will replace the demo pressure source with the calibrated sensor/ADC path, define data validity
and acquisition timing, and revise pressure bands against repeatable idle, cruise, and WOT
baselines. The final button mapping, settings menu, persistent preferences, calibration workflow,
and XIAO configuration will be reviewed when those features are planned.
