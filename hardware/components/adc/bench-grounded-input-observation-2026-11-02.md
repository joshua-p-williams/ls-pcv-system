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

The `adc_error` entries in this run reported `raw_code=-1`. This is distinct from `i2c_error`, which represents a failed bus transaction. A -1 code at a grounded input is one ADS1115 count below zero (187.5 µV at the configured ±6.144 V range). TI documents that ADC offset can produce negative single-ended codes near zero. The source now treats exactly this one-code result as 0 µV while retaining `raw_code=-1` for diagnostics; the resulting 0 V remains outside the provisional FTP sensor output domain and is not converted to pressure. More-negative codes remain ADC faults.

## Follow-up grounded-lead capture

A second capture was supplied on 2026-11-02 while the signal and ground leads were held together for about 15 seconds. The trace shows A0 at 0 µV (`raw_code=0`) continuously from approximately 30.36 s to 54.60 s of the firmware's elapsed-time counter, with seven `adc_error` entries (`raw_code=-1`) mixed into that interval. No `i2c_error` entries appear in this capture. After the leads were removed, the open input rose gradually from 7,875 µV to 218,250 µV over about 7.6 seconds.

This confirms that the grounded connection holds A0 at zero, with occasional one-count negative results at the zero boundary. The post-disconnection rise is an open-input response and is not a meaningful voltage reference. The source capture remains in the private staging workspace; the observations here are a derived summary.

## Post-change grounded-lead retest

After uploading the one-code zero-boundary handling, a new capture was supplied on 2026-11-02. It contains 137 consecutive zero-voltage readings from sample 4456 (`62.440 s`) through sample 6904 (`96.780 s`). Thirteen of those readings retained `raw_code=-1`; all reported `state=voltage_read` and `a0_uV=0`. No `adc_error` or `i2c_error` states appear in the capture. Once the leads were removed, the open input again rose gradually. This verifies the firmware behavior for the observed grounded-input case; it does not establish the source of the ADC's one-count offset, sensor calibration, pressure conversion, filter dynamics, or vehicle operation.

This check demonstrates response near ground only. It does not validate sensor calibration, pressure conversion, filter dynamics, or vehicle operation.
