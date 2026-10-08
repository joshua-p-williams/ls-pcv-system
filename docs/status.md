# Project status

## Current status

This page summarizes the design, available hardware records and remaining work. The [historical context](LS_PCV_SYSTEM_CONTEXT.md) preserves earlier assumptions. Purchases are reported; physical performance remains unmeasured unless a linked record states otherwise. Use the [BOM](../hardware/bom/parts.md) for current procurement details.

- No catch can in the original September 6 planning state; a temporary vented can and bracket were installed September 13 in an earlier project. See the [imported installation record](installation/2026-09-13-catch-can.md). Manifold-vacuum PCV remains incomplete.
- Native restrictor source, 2 mm mesh export, 3/4 mm slicer projects, and two model views imported; an initial PAHT-CF print is reported.
- 3 mm baseline selected. 2 and 4 mm digital variants are present; physical print status and as-built bores remain unconfirmed because supplied notes say both printed and planned/printed.
- The adopted CAD geometry uses symmetric 8 mm internal tapers and a 6.4 mm passage, superseding earlier targets. See the [adopted dimensions and validation](../cad/pcv-restrictor/README.md). Physical performance remains unverified.
- Three EVIL ENERGY check valves purchased for USD 18.04 total; [vendor reference images](../hardware/components/check-valve-relief-valve/README.md) imported. Receipt, modification, and measured relief behavior remain unconfirmed.
- SSLHONG B09NVG35CX converter purchased for USD 13.99; [power specifications and mounting references](../hardware/components/gauge-power-supply/README.md) imported. Not yet received; quantity remains unspecified, and output performance and protection design remain unverified.
- FTP sensor B0CNZ2Q1F2 purchased for a reported USD 7.89 and reported on hand; HiSport 13585316 pigtail B09NVW46W2 purchased for a reported USD 7.99 but not yet received. [Purchase details and reference images](../hardware/components/fuel-tank-pressure-sensor/README.md) imported. Purchased quantities remain unspecified; actual markings, pinout, fit and calibration remain pending.
- ESP32-S3 N16R8 development board purchased for USD 7.99; three XIAO ESP32-S3 boards for USD 21.59 total; three Hosyond GC9A01 TFTs for USD 14.39 total. See [gauge electronics](../hardware/components/gauge-electronics/README.md). The N16R8 board has not yet arrived; XIAO/display receipt remains unconfirmed. Operation remains unverified.
- Ten hiBCTR BSS138 I2C translators purchased for USD 6.88 total; [component, terminal reference and sanitized images](../hardware/components/i2c-level-shifter/README.md) imported. Receipt/operation unconfirmed.
- Three ADS1115 modules purchased for USD 5.98 total; [ADC reference and integration notes](../hardware/components/adc/README.md) imported. Not yet received; chip identity, electrical setup and operation remain unverified.
- Planned firmware environment: PlatformIO, Arduino framework for ESP32, C/C++. ADC conditioning/configuration, pin assignments, firmware, calibration, final display layout, and enclosure remain pending.
- Five sanitized historical installation photographs and a dated installation record have been imported. No schematics or measured logs have been imported yet.

Additional [bench-board references](../hardware/components/esp32-s3-dev-board/bench-board-reference.md) and the Espressif module datasheet are imported. Q04/Q05 are Investigating: product imagery supports a YD-style candidate but does not verify the received board; Q30 now selects native USB, IN-OUT closed, USB-OTG open and L21 output using the official family schematic. Normal rail and load checks remain Q05/Q12; extra photos are not a design prerequisite.

## Next work

Wokwi is [adopted](wokwi-adoption.md) for instructional component-and-wire views and future ESP32 simulation. [L01](../simulation/wokwi/README.md) illustrates the complete intended bench connectivity: all non-USB W01 conductors, explicit series/shunt RC wiring and a whole-USB annotation. DevKitC substitutes for the purchased N16R8; unsupported parts use original generic terminal-only symbols, not physical footprints or functioning models. The online opening workflow includes these definitions; VS Code custom-chip registration remains pending. W01 and sturdy solderable protoboard construction remain authoritative. Firmware/build configuration, functional model selection and simulation scenarios follow actual development.

The [component documentation improvement plan](component-documentation-plan.md) establishes the instructional approach and adopted controller/display split. Initial explanations are in place for the gauge electronics, pressure sensor, ADC, power supply, relief candidate and restrictor; N16R8, XIAO and GC9A01 now have separate component pages, with gauge-electronics retained as the integration guide.

Q17 is Investigating: the [pressure-input conditioning review](../hardware/components/adc/conditioning-review.md) records the selected shared 5 V sensor/ADC architecture, translated I2C and selected built RC filter and remaining validation/protection work. Proceed using listed component identities and explicit engineering assumptions. The sensor is on hand and unmarked; its advertised replacement family and nominal 5 V analog interface are the accepted design basis. Q09/Q10 track sourced terminal/behavior expectations, with physical checks during bring-up rather than an identity investigation. Circuit protection and calibration remain design/test work.

[P04](../hardware/pinouts/ftp-sensor.md) and W01 now use the sourced GM-family A/B/C pigtail map as a working assumption. Q27 selects no analog divider, +/-6.144 V gain and bidirectional I2C translation. Q28 is resolved: hiBCTR BSS138 selected and its terminal labels adopted in W01. Q29 tracks actual pull-ups and bus/power bring-up; Q17 tracks stock R1/C1, response and power/protection review for the selected 470 ohm / 1 uF filter. No physical tests are recorded.

Use the [open-questions register](open-questions.md) for actionable follow-up and W01 dependencies. Q01 selects a complete bench wiring design with phased assembly/testing; Q02 confirms the N16R8 for W01 and XIAO for the finished gauge. Q03 selects one USB source at a time for computer-powered debugging or standalone operation, accepting restart when switching. Q04 physical identification and Q05/Q12 board measurements await delivery. Continue design decisions in parallel, then verify the actual board after receipt. Component pages retain final answers and evidence.

- [x] Document [P06: SSLHONG converter interface](../hardware/pinouts/buck-converter.md) from the supplied vendor references.
- [ ] Verify converter polarity, output/return behavior and the intended USB-C connection; establish load margin, protection and USB programming power handling before harness design.

- [x] Document [P05: GC9A01 display interface](../hardware/pinouts/gc9a01.md) from the supplied vendor images.
- [x] Select display VCC and logic at 3.3 V from the purchased listing (Q31).
- [ ] Check display current/backlight behavior, regulator margin and initialization during phased bring-up.

- [x] Document [P04: FTP sensor and pigtail interface](../hardware/pinouts/ftp-sensor.md) from the available vendor references.
- [ ] Use the sourced GM-family terminal map as the working assumption; check pigtail lead continuity and fit during assembly, then calibrate the sensor.

- [x] Document [P03: ADS1115 module interface](../hardware/pinouts/ads1115.md) from vendor imagery and the preserved TI datasheet.
- [x] Select shared 5 V sensor/ADC power, no divider, +/-6.144 V gain and translated I2C.
- [x] Document the built [RC input filter](../hardware/components/rc-input-filter/README.md), initial 470 ohm / 1 uF values and COND nodes in W01.
- [x] Select the hiBCTR BSS138 translator and map its terminals in W01 (Q28).
- [ ] Check fitted translator/ADC pull-ups, bus levels, communication and power transitions during bring-up (Q29/Q08); record stock R1/C1 and review filter response/power behavior and any additional protection (Q17).
- [ ] Verify ADC module header connectivity, pull-ups and address during bring-up; finalize remaining acquisition settings.

- [x] Document [P02: XIAO ESP32-S3 interface](../hardware/pinouts/xiao-esp32s3.md) using manufacturer references.
- [ ] Confirm XIAO revision, reconcile 3V3 current references and resolve the GPIO budget and USB/external-power behavior before harness design.

- [x] Select native USB, IN-OUT closed, USB-OTG open and L21 nominal 5 V distribution (Q30); W01 power connections are defined.
- [x] Document [P01: bench-board interface](../hardware/pinouts/esp32-s3-devkit.md) from supplied imagery and Espressif references.
- [ ] Verify the received N16R8 board against P01, including power paths and memory-reserved pins, before using W01's proposed GPIO assignments.

- [x] Adopt [diagramming/wiring standards](diagramming-and-wiring-standard.md) and establish the [diagram inventory](../hardware/DIAGRAMS.md).
- [x] Adopt Wokwi and establish [L01](../simulation/wokwi/README.md).
- [x] Extend L01 to complete intended bench connectivity with original visual parts and documented model limitations.
- [ ] Introduce actual firmware and suitable functional models/synthetic inputs before claiming gauge simulation; register compiled custom parts for VS Code when needed.
- [x] Document A04, the [engine PCV overview](../hardware/architecture/pcv-system.md).
- [ ] Review the proposed pressure tap/sensor mount and exact PCV ports in A04.
- [x] Document A01, the [overall gauge architecture](../hardware/architecture/gauge-system.md); electrical interfaces remain planned/TBD.
- [x] Document A02, the [gauge power architecture](../hardware/architecture/power-system.md); supply modes, returns, protection and USB behavior require validation.
- [x] Select shared regulated 5 V for the sensor/ADS1115 and board `3V3` for the translator's low-side reference and compatible peripherals. Q31 selects display VCC/logic at 3.3 V from its listed 3-5 V supply range; rail budgets and operating checks remain open.
- [x] Document A03, the [pressure measurement signal chain](../hardware/architecture/measurement-system.md); calibration, timing, fault thresholds and implementation remain pending.
- [x] Draft [W01: complete bench harness](../hardware/wiring/bench/README.md), including proposed GPIO/ADC configuration, four derived assembly views and a generated connection schedule.
- [x] Render and visually inspect W01 with pinned WireViz dependencies and recorded Graphviz/Python versions.
- [ ] Resolve W01's marked power, module, sensor-terminal, conditioning and wire-sizing gaps before assembly-ready wiring; record physical verification separately.

- [x] Import native restrictor CAD and supplied 3MF variants; document provenance and limitations.
- [x] Adopt imported CAD geometry over earlier written targets.
- [ ] Confirm physical variant status, measured bores, and slicer behavior against the adopted CAD before installation.
- [ ] Record outstanding purchased quantities and arrivals for the pigtail, ADC, power converter and bench board; the sensor is on hand. Record hose/hardware quantities and costs when available.
- [x] Import the earlier catch-can installation history and bracket photos.
- [ ] Verify and document current hose routing, fresh-air path, and catch-can port assignments.
- [x] Select initial ESP32-S3 controllers and GC9A01 display; record purchases and reference images.
- [x] Select ADS1115 external ADC modules and record purchase/reference images.
- [ ] Verify ADC chip identity, voltage compatibility, conditioning, reference/gain/rate settings, and GPIO assignments.
- [ ] Bring up one display and evaluate driver-seat readability before choosing final display size/layout.
- [ ] Verify actual sensor connector terminals before powering hardware.
- [ ] Verify converter output/load behavior and finalize power distribution, fuse, transient protection, USB-power handling, and mounting.
- [ ] Characterize sensor zero, slope, repeatability, and usable range.
- [ ] Measure relief opening/reseating behavior and reverse leakage.
- [ ] Implement and bench-validate pressure reading, zero, and min/max capture.
- [ ] Record repeatable restrictor comparisons using the test plan.

One multifunction button is selected for both controller configurations, with [BetterButton](../firmware/README.md#multifunction-button) as the planned input library. Q15 and Q24 are resolved: click clears min/max, double-click is unassigned and long press opens atmospheric-zero confirmation. Q25 is resolved: zero confirmation defaults to Cancel, click toggles Cancel/Zero, double-click selects and inactivity cancels unchanged, with an atmospheric-equalization reminder. Q23 is resolved: fixed brightness initially, with provision for software configuration and persistent user settings later; Q26 tracks menu and storage design. Display dimming capability remains unverified under Q06. Library integration and physical button/GPIO selection remain pending.

W01 requires a [sturdy movable assembly](../hardware/wiring/README.md#bench-construction-and-retention), using a solderable protoboard carrier, soldered female header sockets and removable male-header modules, with mechanical support and strain relief. Protoboard and male/female header strips are on hand; nominal 2.54 mm pitch is accepted for planning. Q16 is resolved: soldered or secure removable connections may be mixed at assembler discretion. W01 retains responsibility for electrical connectivity and wire requirements; actual terminations and layout belong in the build record.

## Open decisions

Exact ports, hose routing and fittings for the selected PCV flow; relief spring and verified setting; final restrictor diameter; pressure limits justified by measurements; ADC conditioning/configuration; GPIO assignments; final display size/layout; exact firmware environment and library versions; buttons; calibration persistence; fuse/transient protection; grounding; sensor mounting; gauge enclosure; optional logging/Holley integration; repository license.

For each resolved decision, record the rationale, evidence link, and affected configuration in the relevant component document. Keep this page focused on current status rather than session transcripts.
