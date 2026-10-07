# Project status

## Current status

This page summarizes the design, available hardware records and remaining work. The [historical context](LS_PCV_SYSTEM_CONTEXT.md) preserves earlier assumptions. Purchases are reported; physical performance remains unmeasured unless a linked record states otherwise. Use the [BOM](../hardware/bom/parts.md) for current procurement details.

- No catch can in the original September 6 planning state; a temporary vented can and bracket were installed September 13 in an earlier project. See the [imported installation record](installation/2026-09-13-catch-can.md). Manifold-vacuum PCV remains incomplete.
- Native restrictor source, 2 mm mesh export, 3/4 mm slicer projects, and two model views imported; an initial PAHT-CF print is reported.
- 3 mm baseline selected. 2 and 4 mm digital variants are present; physical print status and as-built bores remain unconfirmed because supplied notes say both printed and planned/printed.
- The adopted CAD geometry uses symmetric 8 mm internal tapers and a 6.4 mm passage, superseding earlier targets. See the [adopted dimensions and validation](../cad/pcv-restrictor/README.md). Physical performance remains unverified.
- Three EVIL ENERGY check valves purchased for USD 18.04 total; [vendor reference images](../hardware/components/check-valve-relief-valve/README.md) imported. Receipt, modification, and measured relief behavior remain unconfirmed.
- SSLHONG B09NVG35CX converter purchased for USD 13.99; [power specifications and mounting references](../hardware/components/gauge-power-supply/README.md) imported. Quantity, receipt, output performance, and protection design remain unconfirmed.
- FTP sensor B0CNZ2Q1F2 purchased for a reported USD 7.89; HiSport 13585316 pigtail B09NVW46W2 purchased for a reported USD 7.99. [Purchase details and reference images](../hardware/components/fuel-tank-pressure-sensor/README.md) imported. Quantities and receipt status remain unconfirmed; pinout, fit, and calibration remain pending.
- ESP32-S3 N16R8 development board purchased for USD 7.99; three XIAO ESP32-S3 boards for USD 21.59 total; three Hosyond GC9A01 TFTs for USD 14.39 total. See [gauge electronics](../hardware/components/gauge-electronics/README.md). The N16R8 board has not yet arrived; XIAO/display receipt remains unconfirmed. Operation remains unverified.
- Three ADS1115 modules purchased for USD 5.98 total; [ADC reference and integration notes](../hardware/components/adc/README.md) imported. Chip identity, electrical setup, and operation remain unconfirmed.
- Planned firmware environment: PlatformIO, Arduino framework for ESP32, C/C++. ADC conditioning/configuration, pin assignments, firmware, calibration, final display layout, and enclosure remain pending.
- Five sanitized historical installation photographs and a dated installation record have been imported. No schematics or measured logs have been imported yet.

Additional [bench-board references](../hardware/components/gauge-electronics/bench-board-reference.md) and the Espressif module datasheet are imported. Q04/Q05 are Investigating: product imagery supports a YD-style candidate but does not verify the received board; USB-to-5Vin availability and jumper behavior need checking.

## Next work

Use the [open-questions register](open-questions.md) for actionable follow-up and W01 dependencies. Q01 selects a complete bench wiring design with phased assembly/testing; Q02 confirms the N16R8 for W01 and XIAO for the finished gauge. Q03 selects one USB source at a time for computer-powered debugging or standalone operation, accepting restart when switching. Q04 physical identification and Q05/Q12 board measurements await delivery. Continue design decisions in parallel, then verify the actual board after receipt. Component pages retain final answers and evidence.

- [x] Document [P06: SSLHONG converter interface](../hardware/pinouts/buck-converter.md) from the supplied vendor references.
- [ ] Verify converter polarity, output/return behavior and the intended USB-C connection; establish load margin, protection and USB programming power handling before harness design.

- [x] Document [P05: GC9A01 display interface](../hardware/pinouts/gc9a01.md) from the supplied vendor images.
- [ ] Verify display module revision, supply/logic compatibility and backlight circuitry; resolve current budget, GPIOs and initialization before harness design.

- [x] Document [P04: FTP sensor and pigtail interface](../hardware/pinouts/ftp-sensor.md) from the available vendor references.
- [ ] Identify actual sensor/connector markings and obtain an applicable terminal-function reference; verify cavity-to-lead mapping and fit before powering the sensor.

- [x] Document [P03: ADS1115 module interface](../hardware/pinouts/ads1115.md) from vendor imagery and the preserved TI datasheet.
- [ ] Verify ADC module identity, header connectivity, pull-ups and address; finalize conditioning/configuration before harness design.

- [x] Document [P02: XIAO ESP32-S3 interface](../hardware/pinouts/xiao-esp32s3.md) using manufacturer references.
- [ ] Confirm XIAO revision, reconcile 3V3 current references and resolve the GPIO budget and USB/external-power behavior before harness design.

- [x] Document [P01: bench-board interface](../hardware/pinouts/esp32-s3-devkit.md) from supplied imagery and Espressif references.
- [ ] Verify the received N16R8 board against P01, including power paths and memory-reserved pins, before assigning gauge GPIOs.

- [x] Adopt [diagramming/wiring standards](diagramming-and-wiring-standard.md) and establish the [diagram inventory](../hardware/DIAGRAMS.md).
- [x] Document A04, the [engine PCV overview](../hardware/architecture/pcv-system.md).
- [ ] Review the proposed pressure tap/sensor mount and exact PCV ports in A04.
- [x] Document A01, the [overall gauge architecture](../hardware/architecture/gauge-system.md); electrical interfaces remain planned/TBD.
- [x] Document A02, the [gauge power architecture](../hardware/architecture/power-system.md); supply modes, returns, protection and USB behavior require validation.
- [x] Select the ESP32-S3 board `3V3` output for ADS1115 and compatible 3.3 V peripherals. Board current budget and display supply compatibility remain open.
- [x] Document A03, the [pressure measurement signal chain](../hardware/architecture/measurement-system.md); calibration, timing, fault thresholds and implementation remain pending.
- [ ] Set up and record tested WireViz/Graphviz tooling when the first harness is selected.

- [x] Import native restrictor CAD and supplied 3MF variants; document provenance and limitations.
- [x] Adopt imported CAD geometry over earlier written targets.
- [ ] Confirm physical variant status, measured bores, and slicer behavior against the adopted CAD before installation.
- [ ] Confirm receipt status and outstanding quantities (sensor, pigtail, power converter, and bench board); record hose/hardware quantities and costs when available.
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
