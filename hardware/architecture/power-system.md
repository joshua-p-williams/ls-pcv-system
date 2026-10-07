# Gauge power architecture

A02 describes the planned power paths for the single-channel gauge using either the N16R8 bench board or the XIAO ESP32-S3. The embedded Mermaid views expand [A01](gauge-system.md). Exact pins, protection parts, fuse rating and simultaneous-power handling remain unresolved.

For the XIAO implementation, [P02](../pinouts/xiao-esp32s3.md) documents the manufacturer pin functions, USB-linked VBUS and conflicting regulator-current references. The available peripheral budget and combined-power arrangement remain unresolved.

[P06: converter interface](../pinouts/buck-converter.md) records the vendor-labeled input leads, USB-C output and converter checks needed to resolve these power paths.

The [bench-board evidence record](../components/gauge-electronics/bench-board-reference.md) flags a possible IN-OUT jumper dependency for USB power reaching 5Vin. Verify this under Q05/Q12 before using that header to supply the 5 V sensor branch; Q03's selected USB operating mode does not establish that distribution path.

## Supply and distribution

```mermaid
flowchart TD
    vehicle["Switched vehicle supply"]
    bench["Bench DC supply: setting and limit TBD"]
    chosen["One selected DC source; not a combining circuit"]
    fuse["Input fuse: rating and location TBD"]
    protection["Input protection and filtering: design TBD"]
    converter["SSLHONG converter: nominal 5 V output"]
    railFive["5 V distribution"]
    sensor["FTP sensor: planned 5 V supply"]
    board["Selected ESP32-S3 board: verified power input"]
    railThree["Board-regulated 3.3 V: available capacity TBD"]
    adc["ADS1115 module: planned 3.3 V supply"]
    display["GC9A01 module: supply rail TBD"]
    usb["USB host: programming and possible power"]
    usbBoundary["USB / external-power handling: unresolved"]

    vehicle -.->|"Vehicle mode"| chosen
    bench -.->|"Bench converter mode"| chosen
    chosen ==> fuse ==> protection ==> converter
    converter ==>|"5 V"| railFive
    railFive ==> sensor
    railFive ==>|"Board input arrangement TBD"| board
    board ==> railThree
    railThree ==> adc
    railFive -.->|"Candidate only"| display
    railThree -.->|"Alternative candidate only"| display
    usb -.-> usbBoundary
    usbBoundary -.->|"Mode-specific; verify before connecting"| board

    classDef supply fill:#fff1d6,stroke:#956000,color:#493000
    classDef load fill:#e8f3fc,stroke:#245a81,color:#142c3e
    classDef unresolved fill:#f3f3f3,stroke:#666,color:#333,stroke-dasharray:5 5
    class vehicle,bench,fuse,protection,converter,railFive,railThree supply
    class sensor,board,adc load
    class chosen,display,usb,usbBoundary unresolved
```

**Legend:** thick arrows are intended supply paths; dashed arrows are alternative sources, unresolved interfaces or candidate rails. All are planned, not tested. The display's two candidate arrows are alternatives, not instructions to connect both rails. The source-selection node describes a build/operating choice, not a purchased switch, multiplexer or automatic supply-sharing circuit.

The converter's USB-C output is a power output; it is not the USB programming host shown separately. The selected board is either the N16R8 development board or XIAO, not both. The 3.3 V rail uses the selected board's `3V3` output for the ADS1115 and other compatible 3.3 V peripherals. No separate 3.3 V regulator is selected. Verify the actual header location and usable peripheral current separately for the N16R8 and XIAO boards. This resolves the intended rail source, not the display's supply requirements or the total load budget.

## Returns and reference domains

```mermaid
flowchart LR
    sourceReturn["Selected DC source return"]
    inputReturn["Converter input negative"]
    outputReturn["Converter output return"]
    gaugeReference["Gauge supply return and signal reference"]
    analogReturns["FTP / conditioning / ADC returns"]
    digitalReturns["Controller / display / controls returns"]
    hostGround["USB host ground: programming mode"]

    sourceReturn --- inputReturn
    inputReturn -.-|"Internal relationship: verify actual converter"| outputReturn
    outputReturn --- gaugeReference
    gaugeReference --- analogReturns
    gaugeReference --- digitalReturns
    hostGround -.-|"Additional path via selected board; assess"| digitalReturns
```

**Legend:** solid lines indicate intended return/reference relationships; dotted lines indicate relationships requiring verification. This is a logical reference view, not a prescribed star-ground layout, chassis-bond map, isolated-supply claim or wire routing. Separate analog and digital groups make noise/return-current review visible; they do not specify disconnected grounds.

Use a compatible common reference for sensor, conditioning, ADC and controller as described in the [power component notes](../components/gauge-power-supply/README.md). Verify the converter's input/output return relationship and each board's USB-ground relationship before specifying physical bonds. Select the actual vehicle ground point, return conductors, splice locations and signal routing in the harness design. A USB host can add a return path; assess it with the selected power mode rather than assuming it is isolated.

## Operating modes

| Mode | Intended source | Scope and unresolved condition |
| --- | --- | --- |
| Vehicle operation | Switched vehicle supply through fuse/protection/converter | Normal target; no USB host in this baseline. Exact source, ground point and installed behavior remain unverified. |
| Bench converter evaluation | One selected bench DC source through the converter path | Input voltage/current limit must suit the actual converter and test plan. Verify unloaded output before connecting loads. |
| W01 USB development | Computer USB through the verified board input | Selected operating approach: power plus programming/debugging, with converter disconnected. Full-gauge rail availability and source/load capacity remain subject to Q05/Q12. |
| W01 standalone USB operation | Suitable standalone USB supply through the verified board input | Selected alternative: disconnect computer, change source and restart. Supply selection and full-load capability remain pending; no simultaneous sources. |
| Converter-powered debugging with USB | Converter plus USB host | Unresolved: requires documented board power-path behavior and a selected backfeed/isolation strategy. Not an approved connection configuration yet. |

No onboard battery, direct bench 5 V injection or automatic source switching is selected. Q03 selects alternate USB sources for W01, with restart accepted between modes; see the [bench power requirements](../wiring/README.md#bench-power-requirements). Full-gauge USB operation is intended, not electrically verified. The distribution diagram retains the converter path for vehicle and converter evaluation work; W01 must define its verified USB-fed distribution separately. Check signal connections to unpowered modules when choosing a test configuration.

## Rail and load responsibilities

| Domain | Intended users | Evidence / remaining work |
| --- | --- | --- |
| Vehicle/bench input | Converter through fuse and selected input conditioning | The converter's advertised 8-60 V range is a vendor claim, not automotive transient qualification; verify actual operating conditions. |
| Nominal 5 V | Selected controller's appropriate power input and FTP sensor | Sensor requirements, converter output performance, connectors and board power path need verification. |
| Board-regulated 3.3 V | Controller internally and planned ADS1115 module supply | Verify regulator margin, actual ADC module, I2C pull-ups and logic voltage. |
| Display supply | One GC9A01 module | Exact supply and logic requirements unresolved; do not infer voltage from SPI interface naming. |
| Conditioning and controls | Depends on the chosen circuit | Passive elements need a reference; any active circuitry needs an explicitly selected supply. No extra regulator is selected here. |

The converter's advertised 5 V / 3 A output does not establish available current from a controller board's regulator. The earlier 0.5-1 A input-fuse range remains a preliminary note, not a selection. Component ratings, wire gauge, protection location and startup/operating currents must be reconciled together.

Keep voltage-domain compatibility distinct from supply choice: a planned 5 V sensor needs the [conditioning boundary](../components/adc/README.md) before the planned 3.3 V ADC input. Supply sharing alone does not establish ratiometric measurement or calibration accuracy. See the [sensor record](../components/fuel-tank-pressure-sensor/README.md) and [controller/display notes](../components/gauge-electronics/README.md).

## Design and validation work remaining

1. Identify actual board revisions, power pins/connectors, USB power paths, regulator limits, display requirements and converter return relationships.
2. Measure converter output and rail noise under representative loads, including startup and display activity; record instrument, conditions and configuration.
3. Establish a power budget with measured startup/operating current and margin for the selected board, sensor, ADC and display. Do not reserve expansion capacity based only on the converter label.
4. Select fuse, wiring and input protection/filtering. Reverse-polarity and transient behavior, cranking dropout and recovery remain unresolved.
5. Resolve powered/unpowered signal behavior, grounding and USB backfeed before producing assembly-ready wiring.
6. Document and validate the selected bench configuration before deriving the separate vehicle harness. Follow the [test plan](../../docs/testing/test-plan.md) and retain [measurement records](../../data/README.md).

This architecture records current intent and pending decisions; it does not report electrical tests or define vehicle stop limits. Other diagrams remain selectable through the [inventory](../DIAGRAMS.md).

## Diagram validation

Both views were rendered and visually reviewed with Mermaid CLI 12.0.0 and Node 24.15.0. Checked labels, branching, alternative-path styling and return relationships; no clipping was observed. This validates presentation only. Previews remain private scratch output; the Markdown blocks are the published source.

To reproduce, extract the first and second Mermaid blocks to `power-system-1.mmd` and `power-system-2.mmd` in a scratch directory, then run:

```powershell
npx.cmd --yes --package @mermaid-js/mermaid-cli@12.0.0 mmdc -i power-system-1.mmd -o power-system-1.png --size 2000 -b white
npx.cmd --yes --package @mermaid-js/mermaid-cli@12.0.0 mmdc -i power-system-2.mmd -o power-system-2.png --size 1600 -b white
```
