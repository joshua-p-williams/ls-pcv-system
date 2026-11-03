# FTP Sensor Atmospheric Output Bench Observation

This record captures the first powered check of the FTP sensor through the assembled RC input filter and ADS1115. The sensor pressure port was left open to atmosphere. No pressure conversion or calibration was performed.

## Setup and measurements

The N16R8 bench board ran from USB with the `n16r8_adc` diagnostic firmware. The sensor and ADS1115 shared the nominal 5 V branch and ground. The adopted P04 terminal map was used: sensor A is ground, B is signal, and C is 5 V. Power-off continuity checks measured 0.1 ohm from A to ADC G and from C to ADC V; B to ADC A0 measured 0.471 kΩ, consistent with the nominal 470 ohm series resistor.

Reported multimeter readings with the pressure port open to atmosphere:

| Measurement | Reading |
| --- | ---: |
| Sensor supply, C to A | 4.76 V |
| Sensor output, B to A | 1.474 V |
| ADC input, A0 to G | 1.473 V |

The meter make/model, range and accuracy are not recorded. These readings are reported measurements, not a traceable accuracy comparison.

## ADC capture

The following 16 consecutive diagnostic lines were supplied from the same atmospheric check:

```text
M02_ADC state=voltage_read sample=4530 acquired_ms=63595 raw_code=7853 a0_uV=1472438 pressure=not-tested
M02_ADC state=voltage_read sample=4548 acquired_ms=63847 raw_code=7853 a0_uV=1472438 pressure=not-tested
M02_ADC state=voltage_read sample=4566 acquired_ms=64099 raw_code=7853 a0_uV=1472438 pressure=not-tested
M02_ADC state=voltage_read sample=4584 acquired_ms=64351 raw_code=7853 a0_uV=1472438 pressure=not-tested
M02_ADC state=voltage_read sample=4602 acquired_ms=64603 raw_code=7853 a0_uV=1472438 pressure=not-tested
M02_ADC state=voltage_read sample=4619 acquired_ms=64842 raw_code=7853 a0_uV=1472438 pressure=not-tested
M02_ADC state=voltage_read sample=4637 acquired_ms=65094 raw_code=7854 a0_uV=1472625 pressure=not-tested
M02_ADC state=voltage_read sample=4655 acquired_ms=65347 raw_code=7853 a0_uV=1472438 pressure=not-tested
M02_ADC state=voltage_read sample=4673 acquired_ms=65599 raw_code=7853 a0_uV=1472438 pressure=not-tested
M02_ADC state=voltage_read sample=4691 acquired_ms=65851 raw_code=7853 a0_uV=1472438 pressure=not-tested
M02_ADC state=voltage_read sample=4709 acquired_ms=66104 raw_code=7853 a0_uV=1472438 pressure=not-tested
M02_ADC state=voltage_read sample=4727 acquired_ms=66356 raw_code=7853 a0_uV=1472438 pressure=not-tested
M02_ADC state=voltage_read sample=4745 acquired_ms=66609 raw_code=7853 a0_uV=1472438 pressure=not-tested
M02_ADC state=voltage_read sample=4763 acquired_ms=66861 raw_code=7853 a0_uV=1472438 pressure=not-tested
M02_ADC state=voltage_read sample=4781 acquired_ms=67114 raw_code=7853 a0_uV=1472438 pressure=not-tested
M02_ADC state=voltage_read sample=4799 acquired_ms=67366 raw_code=7853 a0_uV=1472438 pressure=not-tested
```

## Interpretation and limits

The ADC reported 1.472438 V for raw code 7853 and 1.472625 V for code 7854. The observed one-count spread is 187.5 µV at the selected `+/-6.144 V` range. The meter reading at A0 was 1.473 V, close to the code-derived voltage; the available meter specification is insufficient to treat this as an accuracy characterization.

The sensor output (1.474 V) falls within the published 1.3–1.7 V atmospheric reference for the advertised GM sensor family in [P04](../fuel-tank-pressure-sensor/README.md#published-reference-points-for-the-advertised-gm-family). Applying that family reference to the listed aftermarket sensor remains a working assumption. The agreement shows a plausible atmospheric operating point and that the DC signal reached the ADC through the selected filter. It does not validate pressure direction, conversion slope, linearity, accuracy, dynamic filter response, or performance under vehicle conditions. Controlled pressure/vacuum characterization remains Phase 5 work.

Using the unchanged family-default zero of 1.500 V, the shared conversion model would interpret
1.472438 V as approximately `+0.1 inH2O` after rounding to the firmware's tenth-unit representation.
Under the current provisional status boundaries, that calculated value falls just above the upper
danger boundary. This is a model calculation from the atmospheric voltage, not evidence that the
engine has positive crankcase pressure. The family default remains active until the confirmed
atmospheric zero-capture workflow is implemented and deliberately used; the observed 1.472438 V
has not been silently substituted as calibration.
