# P04: FTP sensor and pigtail interface

**Document state: Draft.** This page adopts a sourced GM-family terminal map for the purchased FTP sensor and HiSport pigtail. Applying the family map to the listed replacement is a working assumption. Physical fit and lead continuity are assembly checks; sensor calibration follows bring-up.

## Parts and evidence

| Item | Recorded identity | Evidence and limits |
| --- | --- | --- |
| Sensor | Aftermarket listing B0CNZ2Q1F2; cross-references 16196060 / 16238399 / 12219388 | **Working assumption based on Vendor Listing**: use this replacement family for design. Sensor is on hand and reported unmarked; no further identity check is required absent a discrepancy |
| Pigtail | HiSport listing B09NVW46W2, advertised as 13585316 | **Vendor Listing**. Three-terminal connector and leads pictured; exact housing/terminal identifiers and fit remain unverified |

The [component record](../components/fuel-tank-pressure-sensor/README.md) owns purchase details, source links, dimensions and image provenance. PT2782 and PT2646 are historical cross-references. Use advertised sensor/pigtail compatibility as the prototype basis; a separate authenticity check is not required.

The terminal reference is GM Service Information, **Document ID 1706435, page 19 of 45**, Engine Controls Connector End Views for the 2007 Cadillac Escalade / related C/K applications, preserved in an [externally hosted GM service-document copy](https://forum.efilive.com/attachment.php?attachmentid=8885&d=1282840114#page=19). It depicts the female harness connector, OEM 12059595 / service 88986451, with latch up, A at left and C at right, and assigns A low reference, B pressure signal and C 5 V reference. These reference connector numbers are not additional purchases. The page was visually checked against its table; applying it to the purchased replacement family is an explicit design inference, not a measured pinout. The external document is linked rather than republished.

## Connector views and position record

![Vendor pigtail views](../../media/reference/fuel-tank-pressure-sensor/pigtail-details.jpg)

Use the lower-right **Front** inset as the pigtail **mating-face view**: look into the three terminal openings with the latch above the row. The lower-left **Back** inset shows the wire-entry side. These are angled vendor views, not orthographic connector drawings. Left and right reverse when switching between mating-face and wire-entry views while keeping the latch above the row.

The positions below use the GM female connector mating-face convention. They apply to the pigtail with its latch above the row, not the wire-entry side. A/B/C are adopted reference IDs even if the purchased housing has no readable letters.

| Position, mating face with latch above | Reference cavity ID | Electrical function / domain / direction | Evidence |
| --- | --- | --- | --- |
| Left terminal opening | A | Ground / low reference; return | GM family map; adopted working assumption |
| Center terminal opening | B | Analog pressure output to conditioning | GM family map; adopted working assumption |
| Right terminal opening | C | Nominal 5 V supply input | GM family map; adopted working assumption |

All three pictured leads are light colored. Neither color nor routing in the illustration identifies a function. The advertised lead length is 15 cm (5.9 in); actual length, conductor specification, termination and strain relief must be checked for the harness.

The [sensor illustration](../../media/reference/fuel-tank-pressure-sensor/sensor-dimensions.jpg) includes an angled view into its electrical connector. Opposing mating faces mirror left and right: do not copy the pigtail's left-to-right order directly onto a sensor-face drawing. W01 uses the pigtail reference IDs and view above. Match the latch/key during assembly and label the leads A/B/C after continuity checking.

## Planned electrical interface

The design assumes a three-wire analog sensor. Directions below are relative to the sensor; cavity IDs use the adopted pigtail reference map.

| Required function | Sensor cavity / pigtail lead | Direction | Planned domain and destination | Evidence |
| --- | --- | --- | --- | --- |
| Supply | C | Power input | Nominal 5 V from the gauge supply arrangement | Sourced family map, working assumption |
| Return / low reference | A | Power return and signal reference | Gauge measurement reference shared with conditioning and ADC | Sourced family map, working assumption |
| Pressure output | B | Analog output | Input conditioning, then ADS1115 A0; use 0-5 V as the nominal conditioning design envelope, not a measured pressure transfer function | Sourced function; engineering envelope assumption |

[A02: power architecture](../architecture/power-system.md) uses a nominal 5 V sensor supply as the working assumption shared with the ADS1115; the controller retains 3.3 V logic through an I2C translator. Use applicable family documentation to plan the connection and initial checks; measured sensor behavior follows during bring-up. Do not try terminal permutations to discover the pinout.

The sensor output must pass through the filtering/protection boundary in [A01](../architecture/gauge-system.md). [P03: ADC interface](ads1115.md) defines the ADC-side limits for the selected 5 V ADC domain. The [ADC decision](../components/adc/conditioning-review.md) removes divider scaling; the signal filter remains to be specified. Resolve supply tolerances, loading, return routing and the condition where the sensor is powered while the ADC is off.

[W01](../wiring/bench/README.md#unresolved-interfaces-and-power) uses FTP.A/B/C at the sensor/pigtail assembly boundary. These are the adopted pigtail reference IDs; lead labels and splices are recorded during assembly. Do not assign ESP32 GPIOs to this analog sensor output; the controller interface is through the ADS1115.

## Pressure interface and calibration

[A04: PCV overview](../architecture/pcv-system.md) proposes a pressure tap at the catch-can outlet before the restrictor. The hose, adapter, sealing, retention and sensor mounting remain to be selected. The vendor's 0.45 in pressure-port-area callout does not establish a bore, thread or compatible hose size.

The gauge requires pressure relative to atmosphere, reported in `inH2O` with negative values for vacuum. Confirm how the actual sensor references atmosphere and preserve any required vent path when mounting it; no vent location is identified by the available images. Verify pressure limits and suitability for the intended environment, protect the sensor from liquid oil/water, and evaluate tubing response. Tap pressure may differ from crankcase pressure because of losses in the connecting flow path.

Source atmospheric output and response expectations from documentation for the advertised family; use them as initial assumptions rather than measured calibration. Do not assume zero volts at atmosphere. Use [A03: measurement chain](../architecture/measurement-system.md) and the [test plan](../../docs/testing/test-plan.md) to calibrate the complete sensor/conditioning/ADC chain. Equalize to atmosphere deliberately; do not automatically zero while the engine runs.

## Evidence needed to complete the terminal map

1. Use the listing's replacement-family identity. The received sensor has no markings; further marking searches are unnecessary. Listing views can support connector planning.
2. Use the GM reference and adopted position table above. Resolve an actual mismatch in latch/key orientation before applying power; there is no need to prove OEM identity first.
3. During assembly, with the pigtail disconnected and unpowered, trace each cavity to its free lead by continuity and apply unique lead labels. This maps the leads to the planned functions.
4. Check mechanical fit and retention when the pigtail arrives. Record the as-built orientation and any departure from the planned connection. These checks do not block circuit design.
5. Document a bench setup using the sourced operating expectations. Measure supply and output at atmosphere, then known positive/negative pressures within the planned test range. Record voltage at both sensor output and ADC input, along with configuration and raw conversions. Revisit assumptions if results disagree.
6. Link the terminal evidence, fit checks and calibration results using the [data conventions](../../data/README.md). Update this page, the component record and the harness when measurements establish or change a fact.

The design terminal map is established as a working assumption. Q09 remains for the routine as-built lead/fit check when the pigtail arrives; it does not block conditioning design. Physical response and calibration remain Q10/Q21.
