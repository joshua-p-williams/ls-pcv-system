# Crankcase pressure gauge firmware

Status: planned; no firmware, selected platform, dependencies, or build/test commands yet. Arduino Nano/ATmega328P is a candidate only. Select MCU and display before adding a toolchain or pin assignments.

## Intended first implementation

- Read the actual GM-style FTP sensor through a compatible analog input.
- Apply measured calibration and display signed pressure in `inH2O`.
- Provide deliberate atmospheric zero and min/max reset controls.
- Capture positive and negative peaks independently of display smoothing.
- Detect implausible readings and invalid calibration visibly rather than presenting them as valid zero pressure.

Never automatically zero while the engine runs. Zero requires the pressure port to be equalized to atmosphere. Persistence, warning thresholds, buttons, sampling rate, and filtering parameters remain design decisions.

## Hardware prerequisites

Document verified sensor terminal positions, signal voltage range, MCU ADC limits/reference, display interface, grounding, and power protection. A 5 V sensor supply does not establish compatibility with a 3.3 V MCU ADC. Confirm compatibility before defining a wiring diagram or connecting hardware.

Use calibration of the actual sensor rather than an assumed OE transfer function. Retain raw ADC data and reference information alongside the [calibration records](../data/README.md).

## Implementation boundaries

Keep acquisition, calibration, filtering, peak capture, display, buttons, and persistence separable without creating a framework before it is needed. Optional serial/SD logging and Holley integration can follow the standalone gauge.

When a platform is selected, document tool versions, setup, build, upload, and test commands here. Software checks should cover calibration/sign conventions, invalid inputs, peak preservation, and zero/reset behavior; bench checks must establish actual electrical and timing behavior.
