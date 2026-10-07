# Open questions

This register coordinates follow-up for unresolved hardware checks and design choices. Component and interface pages retain technical details and final decisions; [project status](status.md) summarizes progress. A documented interface is not a verified connection.

## Start here for W01

First resolve **Q01-Q03**: bench scope, controller and power mode. The existing direction uses the N16R8 for bench work; a controller-and-display first build is a proposal, not a selected scope. These choices determine which checks apply.

For a controller-and-display build, focus next on **Q04-Q06, Q12 and Q14-Q16**. Adding the pressure chain also requires **Q07-Q10 and Q17-Q18**. Converter-powered operation adds **Q11 and Q19**. XIAO-specific **Q13** applies only if that board is used. **Q20-Q22** concern operational validation or later vehicle work; they do not all block drafting a bench harness.

Draft W01 can show unresolved work, but assembly-ready connections require the relevant evidence. Tests needing a wired fixture follow verification of its terminal mapping and electrical limits; the register does not require a finished gauge before designing that fixture.

## Design decisions

| ID | Question | How to resolve | Blocks / applies to | Status | Reference / recorded answer |
| --- | --- | --- | --- | --- | --- |
| Q01 | Does W01 begin with controller/display only, or include the ADC and sensor? | Choose the first build scope and explicitly defer excluded connections | W01 scope | Open | [Inventory](../hardware/DIAGRAMS.md) |
| Q02 | Will W01 use the planned N16R8 bench board or a XIAO? | Confirm the board for this harness; retain separate board configurations | W01 controller and pin map | Open | [Gauge electronics](../hardware/components/gauge-electronics/README.md) |
| Q03 | Which power/programming mode will the first bench build use? | Select an A02 mode and identify sources, peripherals powered and programming connection; combined power needs Q19 | W01 power topology | Open | [A02 operating modes](../hardware/architecture/power-system.md#operating-modes) |
| Q14 | Which GPIOs serve SPI, I2C, display control and any buttons or ADC-ready signal? | Allocate after board verification; preserve memory/boot/debug restrictions and record explicit GPIOs in the harness | W01 selected interfaces | Open | [P01](../hardware/pinouts/esp32-s3-devkit.md), [P02](../hardware/pinouts/xiao-esp32s3.md) |
| Q15 | What controls are included, and is display dimming required? | Choose button count/functions and whether dimming belongs in the first build; verify a dimming method before assigning hardware | W01 control wiring and pin budget | Open | [P02 signal allocation](../hardware/pinouts/xiao-esp32s3.md#signal-interfaces-and-project-allocation), [P05](../hardware/pinouts/gc9a01.md) |
| Q16 | What connectors, wire specifications, lengths, retention and return routing will the bench harness use? | Choose construction details from verified interfaces and expected loads; record connector views and conductor identities in W01 | Assembly-ready W01 | Open | [Harness documentation](../hardware/wiring/README.md) |
| Q17 | What conditioning circuit will connect the sensor to ADC A0? | Select scaling, filtering and protection using verified sensor behavior and ADC limits; evaluate tolerances, loading and power-off cases | W01 pressure-chain connections | Open | [ADC preliminary plan](../hardware/components/adc/README.md#preliminary-electrical-plan), [P03](../hardware/pinouts/ads1115.md) |
| Q18 | What ADC address, gain, rate, mode and readiness method will be used? | Choose from verified module configuration and signal range; record polling versus ALERT/RDY, then test fresh-conversion timing | ADC wiring where affected; acquisition bring-up | Open | [P03 configuration](../hardware/pinouts/ads1115.md#i2c-address-and-acquisition-choices) |
| Q19 | How will converter-powered operation handle protection, 5 V distribution and USB programming? | Select fuse/protection, distribution and an explicit host-power/backfeed arrangement using verified board/converter paths | Converter-powered W01; combined-power debugging; W05 | Open | [A02](../hardware/architecture/power-system.md), [P06](../hardware/pinouts/buck-converter.md) |

## Hardware verification

Photos, markings and applicable documentation are useful first evidence. Electrical checks need an identified test setup; preserve measured results separately from vendor claims. The linked interface pages describe the checks in detail.

| ID | Question | How to resolve | Blocks / applies to | Status | Reference / recorded answer |
| --- | --- | --- | --- | --- | --- |
| Q04 | What exact N16R8 carrier/module is present, and does its header map match P01? | Provide clear front/back, module and USB-label views; identify revision and compare header labels with an applicable schematic or continuity evidence | N16R8 W01 terminal mapping | Open | [P01 board identity](../hardware/pinouts/esp32-s3-devkit.md#board-identity-and-evidence) |
| Q05 | What are the N16R8 power paths, programming-port roles and onboard GPIO uses? | Identify regulator, USB bridge, RGB connection and power circuit; verify rails and startup in the selected mode | N16R8 power and GPIO allocation | Open | [P01 verification](../hardware/pinouts/esp32-s3-devkit.md#verification-needed-before-the-bench-harness) |
| Q06 | What are the received display's supply/logic limits and backlight arrangement? | Record front/back markings and header; obtain applicable module documentation, verify circuitry and establish supply/current requirements | Connecting display power and signals; dimming if selected | Open | [P05](../hardware/pinouts/gc9a01.md) |
| Q07 | Is the ADC actually an ADS1115, and does its header match P03? | Record IC/PCB markings and verify header connectivity; corroborate conversion format/timing during bring-up | ADC wiring and acquisition assumptions | Open | [P03](../hardware/pinouts/ads1115.md) |
| Q08 | What pull-ups, ADDR bias and decoupling are fitted to the ADC module? | Inspect/measure unpowered circuitry; verify rail and idle bus levels in a documented setup | I2C/address wiring and Q18 | Open | [P03 power and logic](../hardware/pinouts/ads1115.md#power-logic-and-analog-boundary) |
| Q09 | Which sensor cavities are supply, return and output, and which pigtail leads reach them? | Record actual markings and both mating faces; obtain an applicable terminal reference; trace the disconnected pigtail and confirm fit | Applying sensor power and W01 pressure-chain mapping | Open | [P04 terminal evidence](../hardware/pinouts/ftp-sensor.md#evidence-needed-to-complete-the-terminal-map) |
| Q10 | What supply, output, pressure and atmospheric-reference limits apply to the actual FTP sensor? | Obtain part-specific specifications; after Q09, measure atmospheric output and response within verified limits | Conditioning design and pressure testing | Open | [P04](../hardware/pinouts/ftp-sensor.md) |
| Q11 | Does the converter match its label, and how do its output, returns and USB-C connection behave? | Record actual unit; verify polarity, intended adapter mapping, return relationships and output/load behavior in a suitable fixture | Converter-powered bench build and vehicle power | Open | [P06 verification](../hardware/pinouts/buck-converter.md#verification-before-harness-design) |
| Q12 | Can the selected board and source supply the intended peripheral load? | Establish applicable ratings; measure rails and startup/operating loads progressively, including display activity and controller consumption | Connecting selected loads; final power budget | Open | [A02 rail responsibilities](../hardware/architecture/power-system.md#rail-and-load-responsibilities) |
| Q13 | Which XIAO revision is present, and what regulator and USB/VBUS limits apply? | Record board markings; match a schematic, reconcile the 600/700 mA references and verify relevant power paths | XIAO build only; does not block an N16R8-only harness | Open | [P02 power interfaces](../hardware/pinouts/xiao-esp32s3.md#power-interfaces) |
| Q20 | What initialization/configuration makes the display work correctly and read clearly? | Record driver/version, reset and SPI settings; test colors, rotation, clipping and viewing position | Display bring-up and final display/enclosure choice; not basic header identification | Open | [P05 bring-up](../hardware/pinouts/gc9a01.md#controller-allocation-and-bring-up) |
| Q21 | What calibration and response does the complete measurement chain achieve? | Record known-pressure points, zero/slope/repeatability, acquisition timing, peaks and fault behavior with the exact configuration | Valid pressure readings and vehicle tuning; follows a verified bench fixture | Open | [Test plan](testing/test-plan.md), [A03](../hardware/architecture/measurement-system.md) |
| Q22 | How will the pressure tap, sensor reference/vent, hose and mounting be implemented? | Confirm actual PCV ports and routing; select sealing/retention and protect from liquids while preserving the required pressure reference | Installed pressure measurement; W05 and mounting work | Open | [A04](../hardware/architecture/pcv-system.md), [P04 pressure interface](../hardware/pinouts/ftp-sensor.md#pressure-interface-and-calibration) |

## Maintaining the register

- Keep IDs stable; append new IDs without renumbering existing entries. Grouping and display order may change as dependencies become clearer.
- Use **Open**, **Investigating** or **Resolved**. Mark Investigating when evidence collection or evaluation actually begins. If a question becomes inapplicable, record the scope decision and resolve it with that explanation.
- Record the answer, rationale and evidence in the relevant component/interface document first, then add a brief answer/link here and update status. Retain resolved rows so references to their IDs stay useful.
- A design selection can resolve a decision without proving hardware behavior. Keep associated verification questions open until their own evidence exists.
- Place new photos or source documents in `_staging/inbox/<batch>/` for review and sanitized import. Public answers must link to public project records, not ignored staging files. For measurements, use the [data conventions](../data/README.md).
- Update affected harnesses and [project status](status.md) when answers change their readiness. This register tracks follow-up, not every implementation task or historical assumption.
