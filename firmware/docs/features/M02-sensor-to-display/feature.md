# Feature: Live Sensor-to-Display Measurement Path

## What is this feature?

M02 replaces the synthetic pressure reading used by the first gauge demonstration with a live
reading from the fuel-tank-pressure (FTP) sensor. It carries a pressure measurement from the
sensor through the gauge's measurement path and presents it on the existing display pages. The
work also provides a user-initiated zero-reference adjustment so the installed measurement chain
can correct its pressure offset after installation or relevant maintenance.

The feature is developed and checked on the bench. It uses measured electrical inputs and
controlled pressure/vacuum references to make each part of the measurement path understandable
and testable before the gauge is relied on in a vehicle.

## Why it matters

A gauge is useful only when its displayed value has a traceable relationship to the pressure at
the sensor. Testing the electrical input, pressure conversion, zero adjustment, and display in
stages helps distinguish a wiring or acquisition problem from a sensor or conversion problem. The
same stages make the repository useful to people learning how a pressure measurement system is
built and verified.

The initial status bands provide practical guidance while readings are being understood. They are
not engine-specific failure limits, and pressure by itself cannot identify a mechanical fault.

## What “done” looks like

- The normal live configuration displays readings from the physical FTP sensor rather than the
  synthetic source. The synthetic demonstration remains available as a separate way to exercise
  the display without relying on sensor readings.
- The pressure measurement path can acquire the sensor signal, convert it to signed pressure
  relative to atmosphere, and provide valid readings to the gauge. Negative values mean vacuum;
  positive values mean pressure. Displayed pressure uses `inH2O`.
- Known electrical input voltages are checked against a multimeter so ADC acquisition and voltage
  conversion are verified independently of the pressure sensor.
- The sensor is evaluated against controlled pressure and vacuum readings from the available
  reference instruments. The test record captures reference pressure, sensor output, acquired
  signal, and displayed pressure so conversion behavior and limitations can be reviewed.
- The gauge, 30-second trend, and current-run minimum/maximum pages continue to use the active
  pressure source. Their updates do not depend on a serial monitor being open.
- The live gauge uses the project's initial status bands: lower danger at `−12 inH2O`, lower
  warning at `−8 inH2O`, upper warning at `−3 inH2O`, and upper danger at `0 inH2O`. These are
  research-informed initial operating guidance, not validated engine-damage thresholds. Their
  rationale is maintained in the [FTP sensor component record](../../../../hardware/components/fuel-tank-pressure-sensor/README.md#implications-for-gauge-limits).
- Readings outside the installed sensor chain's valid measurement domain, sensor faults, and
  ordinary warning/danger readings are distinguishable. The gauge does not display extrapolated
  pressure beyond a valid calibrated range.
- A Settings page provides a user-confirmed `Set zero` action. The user is instructed to expose the
  crankcase and sensor port to atmosphere under the defined safe conditions, then wait for a stable
  reading. A successful zero adjustment persists across power cycles; if no valid adjustment is
  stored, the documented default conversion is used. The interface reports successful storage or
  why a capture was rejected.
- Zero adjustment corrects pressure offset only. It does not change the sensor's pressure/voltage
  slope, extend its measurement range, or replace multi-point bench characterization.
- In live mode, long press navigates among the gauge, trend, min/max, and Settings pages. On
  measurement pages, single and double clicks do not change the pressure reading. On Settings,
  single click advances through options and double click selects or confirms the highlighted
  option. The M01 synthetic demonstration retains its temporary click and double-click gestures.
- Automated tests cover pressure conversion, zero-offset behavior, status classification and
  boundaries, invalid/out-of-range handling, and the pressure-history behavior affected by live
  readings. Bench checks separately verify the physical acquisition path and displayed behavior.
- Firmware code and nearby documentation explain each public function, measurement concept,
  assumption, unit, and failure state in accordance with the
  [firmware coding standards](../../coding-standards.md).

## What this feature is deliberately not

M02 does not establish the engine's operating baseline or claim that the initial status bands are
validated limits for this engine. It does not authorize vehicle testing or make the gauge a
standalone diagnostic instrument. It does not provide user editing of warning/danger bands, a
full multi-point calibration procedure, persistent logging, engine-data integration, finished
XIAO deployment, or final enclosure design. It does not claim the sensor or comparison instruments
are certified, or that bench observations prove a specific mechanical condition.

## Related project records

- [M02 brainstorm](brainstorm.md)
- [FTP sensor design and pressure-band rationale](../../../../hardware/components/fuel-tank-pressure-sensor/README.md)
- [ADS1115 component and signal-chain notes](../../../../hardware/components/adc/README.md)
- [Pressure measurement architecture](../../../../hardware/architecture/measurement-system.md)
- [Bench measurement test plan](../../../../docs/testing/test-plan.md)
