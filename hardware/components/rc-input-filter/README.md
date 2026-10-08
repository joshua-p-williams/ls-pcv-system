# RC sensor-input filter

This is a **built component**: a resistor and capacitor assembled on the gauge's protoboard, rather than a purchased module. It filters the FTP sensor signal before the ADS1115 measures it. The sensor and ADC share 5 V; this circuit does not divide their signal voltage or translate I2C logic.

## What an RC filter does

**R** means resistance and **C** means capacitance. A series resistor limits how quickly charge reaches the capacitor; the capacitor stores charge at the ADC input node. Together they make a **low-pass filter**: slow voltage changes pass with little attenuation, while faster changes are reduced.

Electrical noise can enter through sensor wiring or nearby switching electronics. Filtering before conversion reduces some of that noise before it becomes part of a reading. It is different from software display smoothing: an analog filter changes every measurement, including peak capture, so excessive filtering can hide real pressure events. The [measurement architecture](../../architecture/measurement-system.md) keeps additional display smoothing separate from acquisition and peaks.

A capacitor cannot distinguish a real fast pressure change from fast noise. Its value is therefore a tradeoff, not simply something to maximize. The sensor response, pressure tubing and ADC conversion process also affect the complete measurement bandwidth.

## Initial prototype design

| Reference | Selected nominal value | Stock-part guidance |
| --- | --- | --- |
| R1 | 470 ohm series resistor | 1% preferred; 5% acceptable for the initial prototype. A conventional 0.125 W or 0.25 W through-hole part is sufficient for the intended normal signal path, not arbitrary fault exposure |
| C1 | 1 uF capacitor from OUT to GND | Nonpolar film or ceramic; X7R is a practical ceramic choice. Rated at least 10 V; 16/25 V or higher is acceptable if it fits. Effective capacitance, tolerance and voltage dependence affect the filter |

Resistors/capacitors are reported available in existing inventory. The actual stock parts, exact value availability, tolerance, capacitor type/rating and layout have not yet been recorded. These are target design values, not a claim that matching parts have been identified or installed. No purchase is recorded for this circuit.

### Circuit and terminal definitions

```text
Sensor signal -------- R1 470 ohm --------+-------- ADS1115 A0
                                        |
                                      C1 1 uF
                                        |
Common analog ground -------------------+-------- ADS1115 GND
```

| Circuit node | Connection and meaning |
| --- | --- |
| IN | Sensor-side lead of R1; connects to FTP signal B through W_RAW |
| OUT | ADC-side lead of R1 and one C1 lead joined together; connects to A0 through W_A0 |
| GND | Other C1 lead; connects to common analog return through W_ANALOG_GND |

These are electrical node names, not a purchased connector's numbered pins or a prescribed pad arrangement. **IN and OUT are separated by R1; OUT and GND are separated by C1.** R1 and the selected nonpolar C1 have no lead polarity. No second resistor runs from OUT to ground, so this is not the superseded voltage divider. Normal ADC loading can still produce a small voltage drop across R1; calibration includes the complete circuit.

This component page owns the internal R1/C1 circuit. [W01's YAML and connection schedule](../../wiring/bench/README.md) own external connections, wire colors and sectional assembly views. The COND identifier is retained there as this built subassembly's stable name.

### Why these values?

For an ideal low-impedance source and lightly loaded output:

`time constant = R x C = 470 ohm x 1 uF = 0.47 ms`

`cutoff frequency = 1 / (2 x pi x R x C) = approximately 339 Hz`

At the cutoff, amplitude is about 71% of its low-frequency value (-3 dB); cutoff is not a sharp boundary beyond which all signals disappear. The ideal response to a sustained voltage step reaches 90% in about **1.08 ms** and 99% in about **2.16 ms**. These are calculations, not measured sensor/ADC response times or guarantees of peak capture.

This nominal cutoff is above the current 128 SPS conversion-setting candidate and below ten times that setting. TI gives that span as general starting guidance for an external RC filter, and its application example advises filter resistance below 1 kohm to limit loading error. Choosing 470 ohm / 1 uF is a project inference from that guidance, not a TI-prescribed circuit for this sensor. See [ADS1115 sections 9.1.5 and 9.2.2.6](../../datasheets/ads1115-datasheet.pdf) and the [official datasheet](https://www.ti.com/lit/ds/symlink/ads1115.pdf).

The actual cutoff also depends on sensor output impedance, ADC loading and effective capacitance. This one-pole filter plus the ADC's internal filtering is an initial noise-control design, not a guarantee that all unwanted frequencies or aliasing are eliminated. Q18 still needs to establish effective acquisition cadence and its peak-capture limits.

## Building from existing inventory

1. Identify R1 and C1 from stock; record nominal values, tolerances and the capacitor's type/voltage rating. Use the target values where available. For a substitution, recalculate R x C and cutoff, keep the series resistor below 1 kohm as the initial design constraint, and update this record and W01 before calling the substitution the selected design. Do not silently substitute a polarized capacitor for the specified nonpolar part.
2. With power disconnected, place the circuit close to ADS1115 A0. Join the ADC-side R1 lead, C1 lead and A0 connection at OUT. Connect C1's other lead to the local analog reference, with short leads and a return path shared with the ADC and sensor.
3. Solder and support the parts on the carrier, insulate exposed leads and inspect pad/track bridges. Use soldered or secure removable external connections at assembler discretion; label IN, OUT and GND and record the actual pad/connector arrangement.
4. Compare continuity with the node table and W01 schedule. IN-to-OUT should show R1, not a copper short. OUT-to-GND contains a capacitor; an initial charging indication is not the same as a sustained short. Other fitted circuitry can affect in-circuit readings.

## What it does not protect against

This is a signal filter, not a complete overvoltage, reverse-polarity or powered-off isolation circuit. It has no selected clamp diodes, buffer or isolation switch. ADS1115 A0 must remain within its normal GND-to-VDD limits; its +/-6.144 V range does not allow a 6.144 V input on a 5 V supply.

Keep the sensor and ADC on the same 5 V branch. Capacitor charge can remain briefly as power falls, and local supply differences still need review. Q17 retains power-transition behavior and any justified additional input protection; circuit selection does not close those checks. Converter/input protection is separate work in [A02](../../architecture/power-system.md).

Signal filtering and **supply decoupling** are different circuits. C1 connects A0 to ground, not VDD to ground; it does not replace the ADC module's supply bypass capacitor.

## Bring-up and evidence

Follow [W01 phase 4](../../wiring/bench/README.md#phase-4-conditioning-and-sensor) and the [test plan](../../../docs/testing/test-plan.md). Check IN/OUT DC voltage with a known stimulus within ADC limits, then record sensor supply and atmospheric readings after integration. A multimeter can check basic DC behavior but does not establish a millisecond step response or high-frequency rejection. Use suitable acquisition/test equipment when characterizing dynamic response, and record its limitations.

Calibrate with the actual assembled filter. Record R1/C1 and acquisition settings with the [measurement data](../../../data/README.md); inspect noise and timing with display activity. No resistor/capacitor assembly, electrical measurement or dynamic test is recorded here. The nominal calculations define an initial prototype, not a validated release.
