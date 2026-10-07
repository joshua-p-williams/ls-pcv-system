# Crankcase pressure gauge firmware

Status: planned; no firmware, dependencies, or build/test commands yet. The selected direction is **ESP32-S3**, with an N16R8 development board and XIAO ESP32-S3 finished-gauge target. The first display is a **240 x 240 GC9A01 SPI TFT**. Planned development uses VS Code, PlatformIO, the Arduino framework for ESP32, and C/C++. See [hardware selections and purchase records](../hardware/components/gauge-electronics/README.md). This supersedes the earlier Nano/OLED candidates.

Separate PlatformIO environments should eventually share application code while isolating board-specific GPIO/configuration. Exact board definitions, platform/library versions, and pins remain unselected; no environment files are created by this import.

See [A01: overall gauge architecture](../hardware/architecture/gauge-system.md) for acquisition, controller, display and control boundaries, and [A04: PCV system overview](../hardware/architecture/pcv-system.md) for the pressure being observed. Both are planned-system drafts, not verified hardware configurations.

See [A03: pressure measurement signal chain](../hardware/architecture/measurement-system.md) for acquisition/calibration responsibilities, fault handling and separation of peak capture from display smoothing.

Use [P01: bench-board interface](../hardware/pinouts/esp32-s3-devkit.md) when defining the N16R8 environment and GPIO configuration; assignments and actual board identity still require verification.

Use [P02: XIAO interface](../hardware/pinouts/xiao-esp32s3.md) for the XIAO alias/GPIO mapping and pin-budget constraints; the final project assignments remain open.

## Intended first implementation

- Read the actual GM-style FTP sensor through the selected ADS1115 external ADC and required signal conditioning; gain/rate, address, channel configuration, and final analog circuit remain unselected. See the [ADC plan](../hardware/components/adc/README.md).
- Apply measured calibration and display signed pressure in `inH2O`.
- Provide deliberate atmospheric zero and min/max reset controls.
- Capture positive and negative peaks independently of display smoothing.
- Detect implausible readings and invalid calibration visibly rather than presenting them as valid zero pressure.

Never automatically zero while the engine runs. Zero requires the pressure port to be equalized to atmosphere. Persistence, warning thresholds, buttons, sampling rate, and filtering parameters remain design decisions.

## Hardware prerequisites

Document verified sensor terminal positions, signal voltage range, external ADC input limits/reference and bus logic, board power inputs, display supply/interface, grounding, and power protection. I2C is planned for the external ADC and SPI for the GC9A01; neither has assigned GPIOs. A 5 V sensor supply does not establish compatibility with a 3.3 V input. Confirm compatibility before defining a wiring diagram or connecting hardware.

Use calibration of the actual sensor rather than an assumed OE transfer function. Retain raw ADC data and reference information alongside the [calibration records](../data/README.md).

The ADS1115 plan proposes A0 for pressure, a 3.3 V supply, and preliminary signal scaling. Choose a supported conversion rate and measure effective acquisition timing independently of display refresh. Record gain/rate and analog-circuit configuration with calibration; do not hardcode preliminary resistor values as validated hardware.

## Implementation boundaries

Keep acquisition, calibration, filtering, peak capture, display, buttons, and persistence separable without creating a framework before it is needed. Optional serial/SD logging and Holley integration can follow the standalone gauge.

Render a shared gauge state rather than coupling sensor/calibration code to GC9A01 or its 240 x 240 resolution. The initial goal is one pressure display; multiple small displays or a larger approximately 2.1-inch module remain future options pending readability evaluation.

When firmware implementation begins, document exact tool versions, setup, build, upload, and test commands here. Software checks should cover calibration/sign conventions, invalid inputs, peak preservation, and zero/reset behavior; bench checks must establish actual electrical and timing behavior.
