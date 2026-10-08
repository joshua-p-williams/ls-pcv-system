# Diagram and interface inventory

A01-A04 document the planned system architecture. P01-P06 document the controller, ADC, sensor, display and converter interfaces using available product imagery and manufacturer references. P04 adopts a sourced GM-family sensor map as a working assumption; physical verification remains pending throughout. The remaining entries are proposed documentation, grouped by priority and dependency. Follow the [standard](../docs/diagramming-and-wiring-standard.md).

Paths in code are planned destinations, not links to existing artifacts. As documentation is developed, add the artifact link and evidence state; use Proposed, Selected, Draft, Reviewed or Superseded for document progress. Track physical verification separately.

| ID | Priority | Candidate and question answered | Format / planned source | Readiness or dependency |
| --- | --- | --- | --- | --- |
| A01 | 1 | Overall gauge: how do sensor, conditioning, ADC, controller, display, controls and power relate? | Mermaid in [overall gauge architecture](architecture/gauge-system.md) | **Planned design**. Controller alternatives, signal/power boundaries and optional expansion; interfaces TBD |
| A02 | 1 | Power: where do power and returns flow in bench/vehicle and USB modes? | Mermaid in [power architecture](architecture/power-system.md) | **Planned design**. 3.3 V display and W01 USB arrangement selected; source/load validation, converter protection and simultaneous-power handling remain open |
| A03 | 1 | Measurement chain: how does pressure become calibrated readings, peaks and display values? | Mermaid in [measurement signal chain](architecture/measurement-system.md) | **Planned design**. Calibration, validity, zero, peak and display paths; timing/thresholds TBD |
| A04 | 1 | PCV system: where are the fresh-air path, catch can, restrictor, relief and pressure tap? | Mermaid in [PCV system overview](architecture/pcv-system.md) | **Planned design**. Planned routing; proposed sensor tap/mount and exact ports require review |
| P01 | 2 | N16R8 bench-board interface | Markdown [bench-board interface](pinouts/esp32-s3-devkit.md) | **Draft**. Header map and restrictions documented; native USB/jumper/L21 power arrangement selected from family schematic; normal received-board and load checks pending |
| P02 | 2 | XIAO ESP32-S3 interface | Markdown [XIAO interface](pinouts/xiao-esp32s3.md) | **Draft**. Manufacturer pin/alias map documented; revision, regulator-current discrepancy, USB handling and project assignments pending |
| P03 | 2 | ADS1115 module interface | Markdown [ADS1115 interface](pinouts/ads1115.md) | **Draft**. Image-based header map and TI electrical limits documented; fitted IC, board connections, pull-ups, address and configuration pending |
| P04 | 2 | FTP sensor and purchased pigtail interface | Markdown [FTP sensor interface](pinouts/ftp-sensor.md) | **Draft**. Pigtail view convention, planned functions and verification workflow documented; working A/B/C map adopted; as-built lead/fit checks and calibration pending |
| P05 | 2 | GC9A01 display interface | Markdown [GC9A01 interface](pinouts/gc9a01.md) | **Draft**. Seven-position SPI header and listed 3-5 V supply documented; 3.3 V VCC/logic selected; current/backlight, GPIO checks and operation pending |
| P06 | 2 | SSLHONG converter interface | Markdown [converter interface](pinouts/buck-converter.md) | **Draft**. Vendor-labeled input polarity and USB-C power output documented; actual wiring, output/return behavior, USB handling and protection pending |
| W01 | 3 | First bench harness: complete N16R8 single-display measurement-chain wiring with phased assembly/testing | [Guide and diagrams](wiring/bench/README.md), WireViz [source](wiring/bench/bench-harness.yml) | **Draft, not assembly-ready**. Proposed GPIOs and ADC setup; full drawing, four derived section views and connection schedule. Shared 5 V ADC/sensor and translated I2C selected; native USB/L21 takeoff and 3.3 V display selected (Q30/Q31); rail/load checks, translator bring-up, actual interfaces, wire sizing and filter power/protection review remain open; initial 470 ohm / 1 uF filter nodes are documented |
| W02 | 3 | FTP sensor subassembly | WireViz `wiring/bench/ftp-harness.yml` if separate | Verify terminals and conditioning boundary; otherwise include in W01 |
| W03 | 3 | Display subassembly | WireViz `wiring/bench/display-harness.yml` if separate | Confirm module and GPIO choices; otherwise include in W01 |
| W04 | 3 | ADC subassembly | WireViz `wiring/bench/adc-harness.yml` if separate | Confirm I2C, address, analog inputs and conditioning; otherwise include in W01 |
| F01 | 3 | Firmware acquisition/calibration/peak/display data flow | Mermaid in `../firmware/data-flow.md` | After acquisition/configuration decisions; no firmware implementation implied |
| T01 | 3 | Sensor calibration and relief-test workflows | Mermaid in `../docs/testing/test-plan.md` | Add only if a flow clarifies the existing procedure; link real data when available |
| L01 | 4 | Instructional bench component-and-wire illustration | Wokwi [guide](../simulation/wokwi/README.md), [JSON source](../simulation/wokwi/diagram.json) and [PNG preview](../simulation/wokwi/diagram-preview.png) | **Draft, complete intended connectivity**. All non-USB W01 conductors plus internal RC node; USB annotated. DevKitC substitutes for N16R8; unsupported modules use original terminal-only visual parts. Not physical footprints, assembly-ready wiring or working simulation; W01 owns connectivity |
| W05 | 4 | Vehicle gauge harness | WireViz `wiring/vehicle/gauge-harness.yml` | Separate design after source/fuse/ground/routing/connectors and bench findings are resolved |
| L02 | 4 | Gauge enclosure/electronics placement | Draw.io layout or CAD-associated view | After display/enclosure choices; choose destination with CAD work to avoid duplicate drawings |
| S01 | Conditional | Analog conditioning/protection circuit | KiCad under `schematics/` if warranted | Revisit if component-level connectivity needs a real schematic/ERC; not selected now |

## Next documentation

The architecture, P01-P06 interfaces and [W01 harness draft](wiring/bench/README.md) are in place. Use the [open-questions register](../docs/open-questions.md) to resolve the remaining design and interface checks before assembly-ready W01 wiring. W01's section views cover the display, ADC and sensor boundaries without separate competing sources; W02-W04 remain optional subassembly candidates.

[L01](../simulation/wokwi/README.md) now illustrates the complete intended bench wiring using native controller/button/resistor parts and original terminal-only placeholders. Preserve W01's 5 V ADC/sensor, translated I2C and 3.3 V display decisions. Functional models, VS Code custom-chip registration and actual firmware remain future work; see the [model limitations](../simulation/wokwi/README.md#model-coverage-and-next-work) and [adoption decision](../docs/wokwi-adoption.md).

## Inputs already available

Use [gauge electronics](components/gauge-electronics/README.md), [power](components/gauge-power-supply/README.md), [ADC](components/adc/README.md), [sensor](components/fuel-tank-pressure-sensor/README.md), [relief](components/check-valve-relief-valve/README.md), [adopted restrictor](../cad/pcv-restrictor/README.md), [test plan](../docs/testing/test-plan.md) and [current status](../docs/status.md). Existing CAD views and installation photos remain evidence in their component records, not new diagram deliverables.
