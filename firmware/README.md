# Crankcase pressure gauge firmware

M01 implements a synthetic interactive gauge on the N16R8 development board and GC9A01 display.
Its gauge, trend, min/max pages, button gestures, and demo band colors have been exercised on the
physical bench setup. M02 Phases 1-2 add a host-tested measurement core and a checked ADS1115
adapter/acquisition source. Phase 3 adds the `n16r8_adc` bench target, which runs the source and
reports raw codes and calculated A0 voltage without interpreting that input as pressure. It is
ready for physical bring-up, but the assembled ADC bus and input have not yet been verified. The
existing `n16r8` M01 synthetic display target remains unchanged. The Windows Unity host suite passes
30/30 tests; native Linux verification remains pending. Demo pressures and status boundaries are
not measured or validated engine limits.

M01 established the display, interaction, and software boundaries; M02 now extends those boundaries
toward physical measurement. The [M01 feature documents](docs/features/M01-interactive-display-button/)
record scope, research, architecture, plan, and phase evidence. The [LVGL/GC9A01 exploratory
spike](experiments/lvgl-gc9a01/README.md) remains a separate learning reference; the maintained
implementation is under `src/` and `lib/`. The graphics selection is recorded in [ADR 0001](docs/decisions/0001-m01-graphics-stack.md).

Firmware work follows the [Codex spec-driven development workflow](docs/developing-with-codex/README.md)
and [coding standards](docs/coding-standards.md). The [milestone register](docs/milestones/README.md)
records M00 development foundations and M01 interactive display/button work. PlatformIO targets
ESP32-S3 with Arduino; the current environment is the N16R8 bench board. The XIAO ESP32-S3 remains
the planned finished-gauge target and requires its own board configuration and verification.

## How the firmware is organized

`setup()` and `loop()` are entry points supplied by the Arduino framework. `setup()` initializes
the button, display, and event queue once. `loop()` repeatedly handles queued gestures, records
elapsed-time trend samples, and services display animations. One FreeRTOS task polls the button
because an SPI display refresh can occupy the loop task long enough to miss short presses. It sends
only gesture values through a bounded queue; the loop task remains the sole owner of gauge-session
state and LVGL calls.

| Code | Responsibility |
|---|---|
| `src/main.cpp` | Connects concrete board adapters to the gauge core and runs the M01 demo application. |
| `src/adc_voltage_bench_main.cpp` | Phase 3 acquisition-only entry point; reports ADS1115 codes and calculated A0 voltage. |
| `src/board/n16r8/` | W01-based pin aliases and Arduino `Wire` adapter for the N16R8 carrier. |
| `src/measurement/` | Checked ADS1115 register driver, acquisition state machine, and [learning notes](src/measurement/README.md). |
| `src/input/` | Converts BetterButton's GPIO event codes to the shared gesture type. |
| `src/display/` | Uses LVGL for screens/widgets/animation and LovyanGFX for SPI panel setup/pixel transfer. |
| `lib/gauge-core/` | Hardware-independent C++ rules for demo pressure bands, session state, timed history, extrema, and gestures. |
| `lib/better-button/` | Project-authored button gesture library with its source revision and MIT license. |
| `test/test_gauge_core/` | Unity tests for hardware-independent gauge rules. |
| `test/test_ads1115/` | Fake-I2C Unity tests for ADC configuration, conversion, and acquisition faults. |

PlatformIO selects the ESP32 platform, Arduino framework, compiler, and libraries in
[`platformio.ini`](platformio.ini). Arduino supplies embedded startup, GPIO, and time APIs. FreeRTOS
is the task/queue scheduler included by ESP32 Arduino. LVGL is the graphics toolkit; LovyanGFX is
the hardware display driver. Unity runs the core and fake-I2C tests on the development computer,
without communicating with the physical ADC, display, or button.

## Getting started

Open `firmware/` in VS Code and use the PlatformIO extension to build or upload an explicit
environment. The [development environment guide](docs/development-environment.md) explains native
Windows/Linux setup, VS Code and CLI workflows, serial-port selection, and host tests. Run
`pio run -e n16r8` for the M01 display demo or `pio run -e n16r8_adc` for Phase 3 ADC-voltage
bring-up.

The hardware-independent [gauge core](lib/gauge-core/README.md) and ADS1115 state machine have a
PlatformIO `native` test environment. With a host GCC toolchain installed, run `pio test -e native`;
the fake register bus covers software outcomes but does not exercise physical I2C, analog voltage,
GPIO, the display, or ESP32/FreeRTOS integration.

See [A01: overall gauge architecture](../hardware/architecture/gauge-system.md) for acquisition, controller, display and control boundaries, and [A04: PCV system overview](../hardware/architecture/pcv-system.md) for the pressure being observed. Both are planned-system drafts, not verified hardware configurations.

See [A03: pressure measurement signal chain](../hardware/architecture/measurement-system.md) for acquisition/calibration responsibilities, fault handling and separation of peak capture from display smoothing.

Use [P01: bench-board interface](../hardware/pinouts/esp32-s3-devkit.md) and the [W01 proposed controller table](../hardware/wiring/bench/generated/connections.md#proposed-controller-assignments) when defining the N16R8 environment and GPIO configuration. W01 owns the proposed wiring; actual board identity and assignments still require verification.

Use [P02: XIAO interface](../hardware/pinouts/xiao-esp32s3.md) for the XIAO alias/GPIO mapping and pin-budget constraints; the final project assignments remain open.

## Board configurations

The [Wokwi bench illustration](../simulation/wokwi/README.md) documents intended hardware connections with original visual placeholders where no functional model is selected. It is not a runnable project simulation. When simulation support is introduced, point Wokwi at real build outputs, identify synthetic inputs, and keep W01 wiring decisions authoritative; visual stubs do not establish device behavior or physical compatibility.

Use explicit build-time configurations for the N16R8 development board and standard XIAO ESP32-S3. Both use ESP32-S3, so chip identity alone does not identify the carrier or its wiring. The current PlatformIO environment targets N16R8; an XIAO environment has not been added. Select the build/upload target explicitly.

Each configuration must identify its board and harness revision, GPIO mapping, bus assignments, enabled peripherals/controls, and applicable memory, USB and upload settings. Use named functions such as display chip-select and ADC clock rather than scattering board-specific pin numbers through application code. Keep peripheral settings and calibration identity explicit where the assembled hardware differs; calibration is not interchangeable merely because the controller profile changes.

Share measurement, calibration, peak tracking and display behavior across boards. Require a supported board configuration rather than silently falling back to another pin map, and expose the selected configuration in startup diagnostics. Check both builds and validate each on its matching hardware; a successful build does not verify wiring or power compatibility. The XIAO environment and GPIO allocation remain future work.

## Future live-measurement work

The following describes the live gauge direction. M02 has implemented shared pressure
conversion/quality/history rules and the ADS1115 adapter; `n16r8_adc` composes those pieces for
raw-voltage bench checks. The final live gauge composition, physical sensor readings, calibration,
and display integration remain future work. Use the linked hardware records and M02 feature
documents to establish and verify those interfaces.

- Read the actual GM-style FTP sensor through the selected ADS1115 external ADC and required signal conditioning. The shared 5 V supply, +/-6.144 V PGA, 128 SPS single-shot setting, 100 kHz I2C candidate, and ADDR-ground `0x48` address candidate are represented in firmware; physical bus behavior and module identity remain unverified. The initial analog filter is 470 ohm / 1 uF; filter response and any additional protection remain under review. See the [ADC plan](../hardware/components/adc/README.md).
- Apply measured calibration and display signed pressure in `inH2O`.
- Provide deliberate atmospheric zero and min/max reset controls.
- Capture positive and negative peaks independently of display smoothing.
- Detect implausible readings and invalid calibration visibly rather than presenting them as valid zero pressure.

Never automatically zero while the engine runs. Zero requires the pressure port to be equalized to atmosphere. One multifunction button is selected; its initial gesture mapping is selected below; W01 proposes GPIO6 to ground for the N16R8; XIAO allocation and physical switch selection remain open. Persistent user settings are required as described below; storage details, warning thresholds, sampling rate and filtering parameters remain design decisions.

W01 selects native ESP32-S3 USB for programming and diagnostics; the current N16R8 environment
enables its USB CDC serial interface and preserves GPIO19/20. UART remains a separate recovery
option, not the W01 source. Firmware runs the demo without a computer attached, and optional
diagnostics cannot wait indefinitely for host enumeration or a serial monitor.

## Multifunction button

M01 currently uses [BetterButton](lib/better-button/README.md) on the N16R8 GPIO6 input. A click
generates a normal synthetic value, a double-click generates a warning/danger value, and a long
press advances through the three demo pages. This mapping exists only to exercise M01 and does not
define final in-car controls. W01 documents the proposed wiring; XIAO GPIO allocation is separate.

The retained library source is MIT-licensed and its imported revision, timing behavior, and local
adapter are documented in its [README](lib/better-button/README.md). Its 60 ms debounce, 500 ms
double-click window, and 2000 ms long-press values are library settings, not final in-car timing
requirements. Physical N16R8 testing confirmed responsive M01 gesture handling under display load.

The future live gauge interaction decisions below are separate from M01's synthetic demo mapping.
Read button events regularly and handle each returned event once. Keep gesture detection separate
from gauge actions and avoid blocking measurement or peak capture. Q24 selects the following
initial actions in the normal gauge view:

| Gesture | Action |
| --- | --- |
| Click | Clear min/max using the accepted peak-reset behavior; do not change calibration or zero |
| Double-click | Unassigned; no gauge action |
| Long press | Open an atmospheric-zero confirmation prompt; do not change zero merely by opening it |

Q25 selects the atmospheric-zero prompt behavior:

- Long press from the normal gauge view opens the prompt with **Cancel** selected. Opening it does not change zero.
- Show a reminder to equalize the sensor to atmosphere before confirming.
- A single click toggles between **Cancel** and **Zero**.
- A double-click selects the highlighted action: Cancel dismisses unchanged; Zero requests deliberate atmospheric zero using a valid measurement/configuration.
- Inactivity dismisses the prompt without changing zero. The timeout duration remains a firmware tuning parameter.

Keep prompt actions separate from the normal-view peak-reset action; a prompt click must not clear min/max. Verify that the double-click selecting Zero does not first toggle the selection as a single-click action. The interaction does not establish atmospheric conditions automatically or replace calibration/validity checks.

Distinguish normal-view and prompt-state handling, and verify that double-click/long-press recognition does not also invoke a single-click reset. Gesture timing and physical behavior require testing on both controllers. A long press is not proof that the pressure port is at atmosphere. Deliberate zero must retain the atmospheric-condition requirement. Q23 selects the brightness policy below. The initial mapping remains in effect until a later settings-menu design explicitly replaces it.

## Brightness and persistent settings

Use a fixed brightness default for the initial build. Keep the requested brightness/default configurable in software where the verified display hardware supports adjustment. The numeric default, range and mechanism remain TBD. P05/Q06 have not established backlight control on this seven-pin module; a software setting alone cannot establish that capability. Do not add an unverified dimming circuit or GPIO merely to expose a setting, and do not present an unsupported brightness setting as functional.

Plan persistent user settings for later firmware and in-car configuration: explicit initial defaults apply when no valid saved settings exist, and accepted saved choices are restored after power loss/restart. This is the requested EEPROM-style persistence behavior; the storage API/backend and data format remain to be selected during implementation. No external EEPROM component is selected. Keep user preferences distinct from calibration records and board pin configuration; this decision does not settle calibration or peak persistence.

A future single-button settings menu is proposed: long press to enter the menu, single click to advance through items, and double-click to select or edit. It is not yet the finalized interaction mapping. Resolve entry/exit, editing, confirmation, cancellation, save behavior and atmospheric-zero access under Q26, reconciling Q24/Q25 when that menu is adopted. Menu interaction must not suspend acquisition or peak capture.

Persist accepted changes rather than continuously writing unchanged settings. Define validation, format versioning, recovery from invalid/incomplete saved data and restoration of defaults. Verify retention across restart and behavior after interrupted saves on each controller configuration. Firmware and storage implementation remain future work.

## Hardware prerequisites

Before live acquisition, document verified sensor terminals, signal voltage range, ADC input limits,
board power interfaces, grounding, and protection. W01 proposes N16R8 GPIOs for the ADC's I2C and
display SPI; XIAO allocation remains separate. A 5 V sensor supply does not establish compatibility
with a 3.3 V input. Resolve the affected W01 questions and verify circuit behavior before connecting
the sensor or ADC.

Use calibration of the actual sensor rather than an assumed OE transfer function. Each calibration record must define its valid pressure bounds. Firmware may report numeric pressure only within that calibrated domain; outside it, report out-of-range and direction without extrapolating or substituting an endpoint. Keep out-of-range distinct from electrical/ADC faults and from engine warning/danger classification. The advertised GM family reference points are planning evidence, not validated limits for the installed aftermarket sensor. Retain raw ADC data and reference information alongside the [calibration records](../data/README.md).

The ADS1115 and sensor share 5 V, with A0 single-ended and +/-6.144 V range selected for the assumed 0-5 V signal. The ESP32 side remains 3.3 V through an I2C translator. Remove divider compensation from this configuration; 187.5 uV/count is the nominal ADC voltage conversion at the selected range, not a pressure calibration. [W01](../hardware/wiring/bench/README.md#adc-bring-up-candidate) retains provisional address, rate and polling settings. Measure acquisition timing independently of display refresh and record the hardware/configuration with calibration.

## Implementation boundaries

Keep acquisition, calibration, filtering, peak capture, display, buttons, and persistence separable without creating a framework before it is needed. Optional serial/SD logging and Holley integration can follow the standalone gauge.

Render a shared gauge state rather than coupling sensor/calibration code to GC9A01 or its 240 x 240 resolution. The initial goal is one pressure display; multiple small displays or a larger approximately 2.1-inch module remain future options pending readability evaluation.

For each future acquisition feature, document tool versions and reproducible commands in its feature
records and the development guide. Software tests should cover calibration/sign conventions,
invalid inputs, peak preservation, and zero/reset behavior; separate bench checks must establish
electrical behavior and timing.
