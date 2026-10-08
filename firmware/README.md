# Crankcase pressure gauge firmware

Status: planned; no firmware, dependencies, or build/test commands yet. The selected direction is **ESP32-S3**, with an N16R8 development board and XIAO ESP32-S3 finished-gauge target. The first display is a **240 x 240 GC9A01 SPI TFT**. Planned development uses VS Code, PlatformIO, the Arduino framework for ESP32, and C/C++. See [gauge integration guide](../hardware/components/gauge-electronics/README.md) and [procurement BOM](../hardware/bom/parts.md). This supersedes the earlier Nano/OLED candidates.

Separate PlatformIO environments should eventually share application code while isolating board-specific GPIO/configuration. Exact board definitions, platform/library versions, and pins remain unselected; no environment files are created by this import.

See [A01: overall gauge architecture](../hardware/architecture/gauge-system.md) for acquisition, controller, display and control boundaries, and [A04: PCV system overview](../hardware/architecture/pcv-system.md) for the pressure being observed. Both are planned-system drafts, not verified hardware configurations.

See [A03: pressure measurement signal chain](../hardware/architecture/measurement-system.md) for acquisition/calibration responsibilities, fault handling and separation of peak capture from display smoothing.

Use [P01: bench-board interface](../hardware/pinouts/esp32-s3-devkit.md) and the [W01 proposed controller table](../hardware/wiring/bench/generated/connections.md#proposed-controller-assignments) when defining the N16R8 environment and GPIO configuration. W01 owns the proposed wiring; actual board identity and assignments still require verification.

Use [P02: XIAO interface](../hardware/pinouts/xiao-esp32s3.md) for the XIAO alias/GPIO mapping and pin-budget constraints; the final project assignments remain open.

## Board configurations

Use explicit build-time configurations for the N16R8 development board and standard XIAO ESP32-S3, selected through separate PlatformIO environments when firmware is introduced. Both use ESP32-S3, so chip identity alone does not identify the carrier or its wiring. Automatic board detection is not a requirement; the build/upload target must be chosen explicitly.

Each configuration must identify its board and harness revision, GPIO mapping, bus assignments, enabled peripherals/controls, and applicable memory, USB and upload settings. Use named functions such as display chip-select and ADC clock rather than scattering board-specific pin numbers through application code. Keep peripheral settings and calibration identity explicit where the assembled hardware differs; calibration is not interchangeable merely because the controller profile changes.

Share measurement, calibration, peak tracking and display behavior across boards. Require a supported board configuration rather than silently falling back to another pin map, and expose the selected configuration in startup diagnostics. Check both builds and validate each on its matching hardware; building successfully does not verify wiring or power compatibility. Exact environment IDs, GPIOs and implementation remain pending.

## Intended first implementation

- Read the actual GM-style FTP sensor through the selected ADS1115 external ADC and required signal conditioning; the shared 5 V supply and +/-6.144 V range are selected; the initial analog filter is 470 ohm / 1 uF; rate/address candidates, filter response and any additional protection remain under review. See the [ADC plan](../hardware/components/adc/README.md).
- Apply measured calibration and display signed pressure in `inH2O`.
- Provide deliberate atmospheric zero and min/max reset controls.
- Capture positive and negative peaks independently of display smoothing.
- Detect implausible readings and invalid calibration visibly rather than presenting them as valid zero pressure.

Never automatically zero while the engine runs. Zero requires the pressure port to be equalized to atmosphere. One multifunction button is selected; its initial gesture mapping is selected below; physical wiring remains open. Persistent user settings are required as described below; storage details, warning thresholds, sampling rate and filtering parameters remain design decisions.

W01 selects native ESP32-S3 USB for programming/console/debugging; configure the matching USB interface when firmware is introduced and preserve GPIO19/20. UART COM remains a separate recovery option, not the W01 source. Firmware must run the gauge without a computer attached. USB diagnostics must not indefinitely block startup or acquisition while waiting for host enumeration or a serial-console connection.

## Multifunction button

Use one multifunction button for both controller configurations, with [BetterButton](https://github.com/joshua-p-williams/BetterButton) as the planned input library. Its README documents click, double-click and long-press events, a normally open button from GPIO to ground, and an internal pull-up configured by `begin()`. This is the intended interface direction; the physical switch, GPIO, cable and any additional filtering remain to be selected for the harness.

The reviewed [header](https://github.com/joshua-p-williams/BetterButton/blob/master/BetterButton.h) defines 2000 ms long-press, 500 ms double-click monitoring and 60 ms debounce constants. These are upstream reference values, not accepted gauge timing requirements. The README and header were reviewed; implementation audit, ESP32-S3 compatibility and behavior under display/acquisition load remain pending. Pin an exact revision and retain license attribution when integrating; no dependency or source code has been imported yet.

Read button events regularly and handle each returned event once. Keep gesture detection separate from gauge actions and avoid blocking measurement or peak capture. Q24 selects the following initial actions in the normal gauge view:

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

Document verified sensor terminal positions, signal voltage range, external ADC input limits/reference and bus logic, board power inputs, display supply/interface, grounding, and power protection. W01 proposes N16R8 GPIOs for the ADC's I2C and display's SPI interfaces; XIAO allocation remains separate. A 5 V sensor supply does not establish compatibility with a 3.3 V input. Draft wiring identifies unresolved connections with question IDs; confirm compatibility before treating those connections as assembly-ready or attaching hardware.

Use calibration of the actual sensor rather than an assumed OE transfer function. Retain raw ADC data and reference information alongside the [calibration records](../data/README.md).

The ADS1115 and sensor share 5 V, with A0 single-ended and +/-6.144 V range selected for the assumed 0-5 V signal. The ESP32 side remains 3.3 V through an I2C translator. Remove divider compensation from this configuration; 187.5 uV/count is the nominal ADC voltage conversion at the selected range, not a pressure calibration. [W01](../hardware/wiring/bench/README.md#adc-bring-up-candidate) retains provisional address, rate and polling settings. Measure acquisition timing independently of display refresh and record the hardware/configuration with calibration.

## Implementation boundaries

Keep acquisition, calibration, filtering, peak capture, display, buttons, and persistence separable without creating a framework before it is needed. Optional serial/SD logging and Holley integration can follow the standalone gauge.

Render a shared gauge state rather than coupling sensor/calibration code to GC9A01 or its 240 x 240 resolution. The initial goal is one pressure display; multiple small displays or a larger approximately 2.1-inch module remain future options pending readability evaluation.

When firmware implementation begins, document exact tool versions, setup, build, upload, and test commands here. Software checks should cover calibration/sign conventions, invalid inputs, peak preservation, and zero/reset behavior; bench checks must establish actual electrical and timing behavior.
