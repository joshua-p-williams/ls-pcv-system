# Grounded ADC Input Bench Observation

This record captures a near-zero input check using the voltage-only M02 ADC diagnostic firmware. The FTP sensor was not connected and no pressure conversion was performed.

## Reported setup and sequence

The board was running from USB. Following the bench instructions, the signal and common-ground leads were connected together, then removed. The exact contact transition was not instrumented; the firmware trace provides the observed sequence.

## Observed readings

- Before the input reached ground, A0 was around 0.60 V and gradually decreased (for example, 605,250 µV at sample 1162 to 600,563 µV at sample 1504).
- The input then read 0 µV (`raw_code=0`) repeatedly from sample 1522 through sample 2062, apart from intermittent `adc_error` reports.
- After the leads were removed, readings rose from 4,125 µV at sample 2080 to 126,000 µV at sample 2332.
- Three intermittent read errors appear in the supplied trace: `adc_error` with `raw_code=-1` at samples 1540, 1792 and 2062. Valid readings resumed after each.

## Interpretation

The sustained zero readings are consistent with A0 being held at common ground. The gradual voltage rise after the connection was removed is consistent with an open, high-impedance ADC input drifting; an unconnected input is not a valid zero-pressure or zero-voltage reference. The earlier roughly 0.60 V level is likewise an open-input observation, not a circuit offset to subtract.

The `adc_error` entries in this run reported `raw_code=-1`. In the current firmware, an ADS1115 conversion code below zero on the single-ended A0-to-GND input is classified as an invalid ADC input code. This is distinct from `i2c_error`, which represents a failed bus transaction. A -1 code at a grounded input is one ADS1115 count below zero (187.5 µV at the configured ±6.144 V range); it may reflect small input offset/noise around the zero boundary. The firmware currently flags that code rather than treating it as zero, so these reports are an input-boundary handling issue to review, not evidence by themselves of an I²C wiring failure.

## Follow-up grounded-lead capture

A second capture was supplied on 2026-11-02 while the signal and ground leads were held together for about 15 seconds. The trace shows A0 at 0 µV (`raw_code=0`) continuously from approximately 30.36 s to 54.60 s of the firmware's elapsed-time counter, with seven `adc_error` entries (`raw_code=-1`) mixed into that interval. No `i2c_error` entries appear in this capture. After the leads were removed, the open input rose gradually from 7,875 µV to 218,250 µV over about 7.6 seconds.

This confirms that the grounded connection holds A0 at zero when a nonnegative conversion code is returned. The occasional negative one-count result is reported separately by the current software check. The post-disconnection rise is an open-input response and is not a meaningful voltage reference. The source capture remains in the private staging workspace; the observations here are a derived summary.

This check demonstrates response near ground only. It does not validate sensor calibration, pressure conversion, filter dynamics, or vehicle operation.
