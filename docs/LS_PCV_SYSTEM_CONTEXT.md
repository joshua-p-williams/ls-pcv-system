# LS PCV System — Project Brain Dump / Codex Context

> Historical context: this original brain dump preserves early reasoning, targets, and proposals. It is not the current configuration or a wiring/build instruction. Start with the [project README](../README.md), [current status](status.md), and [BOM](../hardware/bom/parts.md). The [restrictor README](../cad/pcv-restrictor/README.md) records actual imported geometry; the [gauge electronics](../hardware/components/gauge-electronics/README.md) and [ADS1115](../hardware/components/adc/README.md) documents supersede the Nano/OLED and internal-ADC proposals. The sections below preserve historical reasoning, with corrections where noted. Restrictor dimensions and taper notes reflect the adopted CAD design.

---

# 1. Project Summary

Repository name:

`ls-pcv-system`

Project purpose:

Develop, document, instrument, test, and tune a custom crankcase ventilation / PCV system for an LS-powered F-body.

The project is larger than a simple catch-can installation. It includes:

- Catch-can plumbing
- Controlled manifold-vacuum evacuation
- Interchangeable fixed-orifice PCV restrictors
- Positive crankcase-pressure relief to atmosphere
- Crankcase-pressure instrumentation
- A custom in-cabin digital pressure gauge
- Firmware for the gauge
- CAD / FreeCAD source models
- 3D-printable parts
- Wiring diagrams and schematics
- Parts research and BOMs
- Pressure calibration
- Test procedures
- Datalogs and tuning results
- Installation photos and documentation

The system should ultimately be tuned from measured crankcase pressure rather than assumptions.

---

# 2. Vehicle / Engine Context

Initial development vehicle:

**2002 Pontiac Trans Am WS6**

Relevant engine/build information:

- Gen III LS-based engine
- Iron block
- 408 CID stroker
- 4.036 in bore
- 4.000 in stroke
- Eagle forged crank
- Eagle forged rods
- DSS forged pistons
- Approximately 10.5:1 compression
- CNC-ported 799 cylinder heads
- BTR "400 NA" camshaft
  - Approximately .636/.636 lift
  - 236/250 duration
- Naturally aspirated
- Stock LS1 intake manifold
- Stock cable throttle body
- Holley Terminator X ECU
- Factory PCM retained for body/gauge functions
- Long-tube headers
- DeatschWerks 78 lb/hr injectors

The engine is relatively fresh and has low mileage since the build.

The PCV project should not assume factory LS airflow or factory crankcase behavior because the engine is significantly modified.

---

# 3. Original Crankcase Ventilation State

Timeline correction: no catch can was present in the original September 6 planning state. A separate earlier project installed a temporary vented catch can on September 13, before this repository was created. See the [installation record](installation/2026-09-13-catch-can.md). The later installed-can descriptions below refer to that subsequent state, not the original configuration.

Before this project, the engine effectively did not have a functional conventional PCV evacuation circuit.

Known plumbing state:

- Passenger valve cover hose connected to throttle-body fresh-air nipple
- Driver-rear valve-cover outlet capped; no rear passenger-side port was found in the September 6 notes
- Intake nipple capped
- No active crankcase evacuation path
- No catch can was installed at this original planning stage; temporary vented operation followed in the September 13 project
- Wispy breather vapor was observed after the September 13 installation, not in this original configuration
- Oil cap has shown some looseness/wiggle behavior

The goal is to convert this into a controlled vacuum-assisted PCV system.

---

# 4. Overall PCV System Concept

Normal operating flow:

```text
CRANKCASE
    |
    v
CATCH CAN
    |
    v
FIXED PCV RESTRICTOR
    |
    v
INTAKE / MANIFOLD VACUUM
```

Separate positive-pressure relief path:

```text
CRANKCASE / CATCH CAN
        |
        v
LOW-CRACKING CHECK / RELIEF VALVE
        |
        v
ATMOSPHERE
```

The normal PCV path should operate under manifold vacuum.

The atmospheric relief valve is not intended to regulate normal vacuum. It exists so that if manifold vacuum disappears under heavy load and blow-by causes crankcase pressure to become positive, the system has a second escape path.

The ideal relief-valve cracking pressure is very low.

Target relief-valve range:

- Approximately 0.08–0.12 PSI
- Approximately 2.2–3.3 inH2O

A 0.1 PSI cracking valve corresponds to approximately 2.77 inH2O.

---

# 5. Crankcase Pressure Targets

Crankcase pressure should be discussed in **inches of water column (inH2O)** rather than normal automotive PSI because the desired pressures are extremely small.

Current practical targets for this custom system:

| Operating condition | Desired behavior |
|---|---|
| Hot idle | Small controlled vacuum |
| Light cruise | Small controlled vacuum |
| High manifold vacuum / decel | Avoid excessive vacuum |
| Heavy load | Vacuum moves toward 0 inH2O |
| WOT | Near atmospheric; small positive excursions possible |
| Positive-pressure relief | Begin opening before significant positive pressure develops |

Current rough working target range for normal idle/cruise:

**approximately -3 to -8 inH2O**

This is a tuning target, not a factory GM specification for this engine.

Under WOT, manifold vacuum disappears while blow-by increases. The crankcase may approach atmospheric pressure or become slightly positive.

A backup relief valve should prevent large positive pressure.

The previously purchased 0.5 PSI check valve would not begin to crack until approximately:

**+13.8 inH2O**

That is considered too high as the preferred cracking point, although it could still serve as an emergency-only relief.

---

# 6. PCV Restrictor Design

A custom inline restrictor has been designed in FreeCAD and printed.

The restrictor is intended to be interchangeable so multiple orifice sizes can be tested without redesigning the full PCV system.

Initial restrictor variants:

- 2.0 mm
- 3.0 mm
- 4.0 mm

Expected baseline:

**3.0 mm**

Potential future intermediate sizes:

- 2.5 mm
- 3.5 mm

Restrictor flow area changes significantly with diameter:

| Diameter | Area | Relative area vs 3 mm |
|---:|---:|---:|
| 2.0 mm | 3.14 mm² | 44% |
| 3.0 mm | 7.07 mm² | 100% |
| 4.0 mm | 12.57 mm² | 178% |

Therefore the 2 / 3 / 4 mm parts represent large tuning steps.

---

# 7. Adopted Restrictor Dimensions

The supplied FreeCAD geometry defines the current restrictor design. The dimensions below and sections 9-11 have been reconciled to that model, superseding the earlier written targets. See the [restrictor design and historical comparison](../cad/pcv-restrictor/README.md). These are nominal CAD dimensions, not measurements of printed parts.

| Feature | Adopted CAD dimension |
|---|---:|
| Metering/restrictor ID | 3.0 mm baseline; 2.0 / 4.0 mm comparison meshes |
| Straight restrictor length | 3.0 mm |
| Overall length | 67.0 mm |
| Maximum center body OD | 14.0 mm |
| Main and barb straight passage ID | 6.4 mm |
| Converging taper 6.4 -> 3 mm | 8.0 mm long |
| Expanding taper 3 -> 6.4 mm | 8.0 mm long |
| Separate internal passage transition | None |
| Barb nominal/root OD | 9.8 mm |
| Barb retention crest OD | 10.9 mm |
| Barb tip / lead-in OD | Approximately 8.716 mm |
| Barb tip taper length | 3.0 mm |
| Straight root/clamp section | 15.0 mm each end |
| Tip-to-body-taper axial span | 19.0 mm each end; not measured hose engagement |
| External body taper 14 -> 9.8 mm | 3.0 mm each end |

Actual hose engagement, retention, finished bores, and performance require physical verification.

---

# 8. Restrictor Geometry Rationale

## Metering section

The calibrated restriction is intended to be:

**3.0 mm ID x ~3.0 mm long**

It is intentionally a short orifice rather than a long 3 mm tube.

The short section concentrates the pressure drop at a known point and avoids adding unnecessary long-tube friction.

For dimensional accuracy, the 3 mm throat may be printed slightly undersized and finished with a 3.00 mm drill bit.

---

# 9. Adopted Internal Tapers

The adopted CAD has symmetric internal tapers. For the 3 mm baseline:

- upstream contraction: 6.4 -> 3.0 mm over 8.0 mm
- downstream expansion: 3.0 -> 6.4 mm over 8.0 mm

There is no separate 8-to-6 mm internal transition. The earlier proposal for a 15 mm exit diffuser was not implemented and is superseded by the adopted CAD design. The [component comparison](../cad/pcv-restrictor/README.md) preserves the earlier dimensional targets.

The short throat remains the intended metering section. Adopting this geometry does not establish pressure recovery, flow rate, or suitability; compare the variants using measured crankcase pressure.

---

# 10. Barb Design

The throttle-body / PCV hose being targeted is approximately:

**3/8 in ID**

3/8 in = 9.525 mm.

An existing fitting measured approximately 9.8 mm OD and was used as a physical reference.

Barb design:

- nominal/root OD: ~9.8 mm
- retention crest: 10.9 mm
- hose-start tip: approximately 8.716 mm
- straight root/clamp section: 15.0 mm each end
- tip-to-body-taper axial span: 19.0 mm each end; actual hose engagement remains unmeasured

The hose should push over the smaller tapered tip, stretch over the retention crest, and settle around the 9.8 mm body.

The hose clamp should sit behind the retention crest on the straight clamp land.

Sharp barb edges should be avoided.

Use small chamfers/radii where possible so the barb does not shave or cut the inside of the hose.

---

# 11. Adopted Barb Bore and Wall Thickness

The adopted main and barb passage is **6.4 mm ID**, with a **9.8 mm root OD**. Nominal radial wall thickness at the straight root is:

`(9.8 - 6.4) / 2 = 1.7 mm`

The passage connects directly to the 8 mm-long internal throat tapers; there is no separate main-to-barb bore transition. The earlier 6 mm barb bore and 1.9 mm wall calculation are superseded.

This is a geometric calculation, not a strength or flow validation. Printed dimensions, material behavior, hose retention, and sealing require physical testing.

---

# 12. Restrictor Material

Printed material:

**QIDI PAHT-CF**

This is a high-temperature carbon-fiber reinforced nylon.

Reasons for selection:

- under-hood temperature capability
- good mechanical stiffness
- engine-bay durability
- resistance to vibration
- compatibility with a small structural hose fitting

The first restrictor has already been printed in QIDI PAHT-CF.

No cosmetic post-processing is considered necessary.

Recommended post-processing only when required:

- remove loose stringing/fuzz
- inspect internal passages
- lightly deburr hose-entry edges
- verify the metering orifice
- finish the calibrated hole carefully with the correct drill bit if needed

Do not coat the internal passage.

Do not use acetone smoothing.

Do not anneal unless a specific dimensional/structural reason arises, because dimensional accuracy is important.

---

# 13. Print Orientation / Printability Notes

Preferred printing orientation:

**vertical, with the axis of the fitting aligned with Z**

Reasons:

- keeps internal circular passages concentric
- keeps the 3 mm bore vertical
- avoids support inside the airflow path
- better bore quality than printing horizontally

Design-for-printing considerations:

- use self-supporting external tapers where possible
- avoid horizontal undersides on barb ridges
- use approximately 45° or gentler transitions
- consider a brim due to the small footprint
- do not place support material inside the airflow passage
- inspect nipple/body transitions for layer separation

The printed part geometry has already been visually reviewed and appears structurally reasonable.

---

# 14. Catch Can

Catch can installed during the earlier September 13 project, in temporary vented mode (manifold-vacuum PCV not yet connected):

**EVIL ENERGY 300 ml baffled catch can with breather capability**

Known project plumbing/material details:

- 1/2 in SAE 30R7 hose has been used in the system
- catch can has a top M16 x 1.5 location used for breather/relief concepts
- custom bracket installed
- bracket attaches to passenger-side cylinder head
- aluminum angle and steel bracket components used
- pop rivets used in bracket construction
- M10 x 1.5 engine/head mounting hardware used
- thread locker applied

The catch can is intended to remain central to the system and collect oil mist before intake vacuum.

---

# 15. Atmospheric Relief Valve

A cheap EVIL ENERGY check valve has been ordered for use as a possible positive-pressure relief.

Known construction from product exploded diagram:

- aluminum body
- threaded/serviceable two-piece body
- NBR O-ring
- NBR gasket
- black poppet
- 304 stainless compression spring

Published opening pressure:

**0.5 PSI**

Conversion:

**0.5 PSI ~= 13.8 inH2O**

This cracking pressure is higher than desired.

Preferred relief cracking pressure:

**~0.1 PSI ~= 2.77 inH2O**

Potential modification strategy:

1. Disassemble one valve.
2. Measure spring:
   - free length
   - outside diameter
   - wire diameter
   - active coils
   - installed/compressed length
3. Prefer replacing the spring with a much lighter spring.
4. Avoid blindly cutting many coils.
5. Test cracking pressure with a water manometer.

A springless experiment may be useful only as a bench test.

A permanent springless valve is not automatically trusted because vibration and orientation may allow the poppet to rattle open.

Important direction:

```text
CATCH CAN -> CHECK VALVE -> ATMOSPHERE
```

The valve must be oriented so crankcase vacuum pulls the valve closed.

---

# 16. Crankcase Pressure Measurement Strategy

A conventional 0–15 PSI automotive pressure sensor is not appropriate.

The expected useful range is only a few inches of water.

Chosen sensor strategy:

**GM-style fuel tank pressure (FTP) sensor**

Target sensor family:

**GM 16238399-style**

Reasons:

- 3-wire sensor
- 5 V reference
- analog output
- differential pressure relative to ambient
- designed for very low positive and negative pressure
- measurement range is appropriate for crankcase-pressure monitoring

A cheap aftermarket sensor is acceptable because the system will be calibrated manually rather than relying blindly on OE transfer-function accuracy.

Chosen inexpensive candidate:

Amazon-compatible sensor cross-referencing:

- 16238399
- 16196060
- 12219388

One candidate already considered:

`B0CNZ2Q1F2`

The gauge should zero/calibrate the actual sensor in use.

---

# 17. FTP Sensor Connector

A separate pigtail is required.

Chosen compatible connector family:

- GM 13585316
- PT2782
- PT2646

A HiSport PT2782 pigtail was identified as compatible.

Do not trust aftermarket wire colors.

The final pinout must be verified by terminal position before applying power.

Signals required:

- 5 V reference
- sensor ground
- analog signal

---

# 18. Dedicated Gauge — High-Level Concept

Current hardware: the current selection is an ESP32-S3 N16R8 bench board, XIAO ESP32-S3 finished-gauge target, initial GC9A01 round SPI TFT, and planned external ADC. PlatformIO with the Arduino framework is the planned toolchain. See [gauge electronics](../hardware/components/gauge-electronics/README.md). Nano/OLED candidates, conceptual I2C display wiring, and statements that the initial MCU/display are undecided later in this historical file are superseded; they are not current wiring instructions.

The crankcase pressure gauge will be independent of the Holley Terminator X.

Reason:

A dedicated in-cabin display is desirable for direct observation and tuning.

The Holley may eventually also receive the signal for datalogging, but that is not required for the first implementation.

Likely gauge components:

- microcontroller
- small OLED display
- GM FTP sensor
- stable regulated 5 V supply
- pushbutton(s)
- custom enclosure
- custom firmware

Initial microcontroller concept:

**Arduino Nano / ATmega328P-compatible board**

This is not yet locked in.

ESP32 or another microcontroller may also be considered later.

Do not unnecessarily lock the repository/firmware structure to Arduino until hardware is finalized.

---

# 19. Gauge Display Concept

Desired display should show at minimum:

```text
CRANKCASE

 -3.4
 inH2O

MIN -6.2
MAX +0.8
```

Useful features:

- large signed live pressure
- unit: inH2O
- peak positive pressure
- peak negative pressure
- zero function
- optional reset peak button
- optional warning indication when positive pressure exceeds target
- optional bar graph
- optional logging later

The user should not need to watch the gauge during a WOT pull.

Peak positive pressure is especially important.

Example workflow:

1. Reset peak.
2. Perform pull.
3. Lift throttle.
4. Read stored maximum positive pressure.
5. Compare restrictor configurations.

---

# 20. Gauge Calibration Concept

The gauge should support a zero/calibration workflow.

At atmospheric pressure:

- engine off
- pressure port open/equalized to atmosphere
- press ZERO
- store current sensor ADC value as 0.0 inH2O

Preferred additional calibration:

Use a simple U-tube water manometer to generate known differential pressure.

Example calibration points:

- 0 inH2O
- +5 inH2O
- -5 inH2O

or similar safe points.

Record ADC counts / voltage.

Use measured slope rather than blindly trusting a generic sensor equation.

This is important because a cheap aftermarket FTP sensor may have offset/slope variance.

---

# 21. Gauge Power Supply

An automotive power converter has been ordered.

Ordered converter:

**SSLHONG DC 8–60 V input -> 5 V output, 3 A USB-C buck converter**

Amazon product:

`B09NVG35CX`

Reasons for selection:

- very wide input range
- 8–60 V input gives better automotive transient margin than 8–35 V versions
- regulated 5 V output
- IP67 / potted style
- listed protection features:
  - over-temperature
  - short circuit
  - over-current
  - overload
- much more output current than actually required

The gauge will likely use only a few hundred mA.

Potential inconvenience:

- output is USB-C
- may require USB-C breakout/pigtail for clean integration

Potential additional input protection:

- small fuse
- TVS diode
- input capacitor

A TVS diode may still be added even though the converter supports up to 60 V input.

Proposed power path:

```text
SWITCHED 12 V
    |
    v
SMALL FUSE
    |
    v
OPTIONAL TVS / INPUT FILTER
    |
    v
SSLHONG 8–60 V -> 5 V BUCK
    |
    +----> MICROCONTROLLER
    |
    +----> OLED
    |
    +----> FTP SENSOR
```

Using the same regulated 5 V supply for the sensor and microcontroller ADC is desirable because the measurement can be largely ratiometric.

---

# 22. Proposed Gauge Wiring

Conceptual wiring:

```text
             SWITCHED 12V
                  |
                  v
             SMALL FUSE
                  |
                  v
        8–60 V -> 5 V CONVERTER
                  |
           +------+------+
           |             |
           v             v
        MCU 5V       FTP SENSOR 5V
           |
GROUND ----+------------- FTP SENSOR GROUND
           |
ANALOG IN <-------------- FTP SENSOR SIGNAL

MCU I2C SDA ------------ OLED SDA
MCU I2C SCL ------------ OLED SCL
5V / GND ---------------- OLED POWER
```

If using an Arduino Nano-class board:

- A0 could be FTP signal
- A4 could be SDA
- A5 could be SCL

This is provisional.

---

# 23. Potential Sensor Placement

Sensor can be:

- mounted underhood near the catch can, with electrical wiring through the firewall
- or mounted inside the cabin with a pressure hose routed inside

Preferred general direction:

**mount the pressure sensor remotely and protect it from direct liquid oil exposure**

The pressure source should represent actual crankcase/catch-can pressure upstream of the PCV restrictor.

Do not measure on the intake side of the restrictor if the goal is crankcase pressure.

---

# 24. Repository Structure

Initial proposed repository layout:

```text
ls-pcv-system/
├── README.md
├── .gitignore
│
├── cad/
│   ├── pcv-restrictor/
│   │   ├── README.md
│   │   ├── source/
│   │   │   └── pcv-restrictor.FCStd
│   │   └── exports/
│   │       ├── stl/
│   │       └── step/
│   └── gauge/
│
├── hardware/
│   ├── bom/
│   ├── schematics/
│   └── datasheets/
│
├── firmware/
│   └── README.md
│
├── docs/
│   ├── research/
│   ├── installation/
│   └── testing/
│
├── data/
│   ├── calibration/
│   └── logs/
│
└── media/
    ├── photos/
    └── diagrams/
```

Guiding principle:

The repository is system-oriented, not software-oriented.

CAD, hardware, firmware, research, testing, and measured data are peers.

Firmware is only one part of the project.

---

# 25. Root README Intent

The root README should explain:

- project purpose
- vehicle context
- overall PCV architecture
- project goals
- major components
- current design state
- directory structure
- development status
- warning that dimensions/tuning are specific to this engine unless otherwise stated

The repository should read like an engineering notebook and implementation project, not merely a code repository.

---

# 26. CAD Repository Rules

Native CAD source is authoritative.

For each designed component:

```text
cad/<component>/
├── README.md
├── source/
└── exports/
```

Example:

```text
cad/pcv-restrictor/
├── README.md
├── source/
│   └── pcv-restrictor.FCStd
└── exports/
    ├── stl/
    └── step/
```

Rules:

- commit `.FCStd`
- do not ignore native CAD source
- exported STL / STEP may also be committed when intentionally generated
- README should explain design intent and dimensions
- CAD should not rely solely on filenames to explain design revisions

---

# 27. Suggested `.gitignore`

Current recommended baseline:

```gitignore
# FreeCAD backups
*.FCStd[0-9]*
*.FCBak
*.FCbak

# OS
.DS_Store
Thumbs.db
Desktop.ini

# Temporary
*.tmp
*.temp
*~
~$*

# Visual Studio
.vs/
*.user
*.suo

# .NET
bin/
obj/

# Python
__pycache__/
*.py[cod]
.venv/
venv/

# Embedded
build/
out/
.pio/

*.elf
*.hex

# IDE
.idea/
```

Do not globally ignore:

- `.FCStd`
- `.stl`
- `.step`
- `.csv`
- `.log`
- `.bin`
- images

Those may be intentional engineering artifacts in this project.

---

# 28. Recommended Initial Files

At minimum, initial repository should contain:

```text
README.md
.gitignore
cad/pcv-restrictor/README.md
cad/pcv-restrictor/source/pcv-restrictor.FCStd
```

Optional early files:

```text
hardware/bom/parts.md
docs/research/pcv-theory.md
docs/testing/test-plan.md
firmware/README.md
```

---

# 29. BOM / Parts Tracking

A BOM should be created early.

Recommended fields:

| Field | Purpose |
|---|---|
| Category | Catch can / sensor / connector / power / hose / electronics |
| Part | Human-readable name |
| Manufacturer | Vendor |
| Part number | Manufacturer/OE number |
| Source | Amazon / Summit / etc. |
| Status | Planned / ordered / received / installed |
| Qty | Quantity |
| Cost | Purchase cost |
| Specs | Key technical values |
| Purpose | Why it exists |
| Notes | Fitment, issues, calibration, alternatives |

Known items to capture:

- EVIL ENERGY 300 ml catch can
- PCV hose
- QIDI PAHT-CF filament
- 2 mm restrictor
- 3 mm restrictor
- 4 mm restrictor
- EVIL ENERGY 0.5 PSI check valve
- GM 16238399-compatible FTP sensor
- PT2782 / 13585316 sensor pigtail
- SSLHONG B09NVG35CX 8–60 V to 5 V converter
- Arduino/Nano-class microcontroller if chosen
- OLED display if chosen
- button(s)
- enclosure hardware
- fuse
- TVS diode if added
- wiring/connectors

---

# 30. Documentation Philosophy

This project should preserve **why**, not merely **what**.

For important design decisions, document:

- original problem
- alternatives considered
- chosen approach
- calculation
- dimensions
- assumptions
- implementation
- test result
- follow-up decisions

Examples:

- Why 3 mm baseline?
- What wall thickness does the adopted 6.4 mm passage provide?
- What measurements support retaining or changing the adopted symmetric tapers?
- Why GM FTP sensor instead of a conventional PSI sensor?
- Why low cracking pressure relief?
- Why separate gauge from Holley?
- Why PAHT-CF?

The repository should remain useful months or years later.

---

# 31. Testing Plan — High Level

Restrictor testing should be performed under repeatable operating conditions.

Suggested test points:

1. Engine off / atmospheric zero
2. Cold idle
3. Fully warm idle
4. Light cruise
5. Moderate cruise
6. Deceleration / closed throttle
7. Moderate acceleration
8. High load
9. WOT pull when safe
10. Hot restart / idle

For each restrictor:

- 2.0 mm
- 2.5 mm if made
- 3.0 mm
- 3.5 mm if made
- 4.0 mm

Record:

- crankcase pressure
- RPM
- manifold pressure if available
- throttle position if available
- oil in catch can
- oil consumption
- idle quality
- leaks
- vapor behavior
- positive pressure peak
- negative pressure peak

---

# 32. Gauge Firmware — Proposed Functional Requirements

Firmware should likely implement:

## Required

- initialize display
- read analog FTP sensor
- convert ADC reading to pressure
- signed inH2O display
- zero calibration
- live pressure update
- track minimum pressure
- track maximum pressure
- reset min/max

## Strongly desired

- stored calibration constants
- startup sensor sanity check
- display warning for implausible sensor values
- configurable positive-pressure warning
- configurable excessive-vacuum warning
- simple debounce for buttons
- smoothing/filtering that does not hide short pressure spikes

## Possible later additions

- logging to serial
- logging to SD
- Bluetooth
- CAN integration
- Holley input/output
- configurable units
- trend graph
- backlight/brightness control
- startup splash/status
- firmware version display

---

# 33. Pressure Filtering Considerations

Do not over-filter the pressure signal.

The gauge is being used partly to capture short positive-pressure excursions under load.

A long averaging window could hide important events.

Potential approach:

- fast raw sample rate
- light moving average or low-pass filter for the live number
- peak detector based on less-filtered or unfiltered samples
- optional separate display smoothing vs peak-capture path

Example:

```text
ADC SAMPLE -> CALIBRATION -> PRESSURE
                         |-> light smoothing -> live display
                         |-> peak detector -> max/min storage
```

---

# 34. Calibration Data Storage

Calibration should be stored explicitly.

Possible representation:

```text
zero_adc
positive_cal_adc
positive_cal_pressure
negative_cal_adc
negative_cal_pressure
```

Or linear calibration:

```text
pressure = slope * adc + intercept
```

If using voltage:

```text
pressure = slope * volts + intercept
```

If power supply/reference is ratiometric, ADC counts may be preferable.

Calibration data should be documented under:

`data/calibration/`

Potential files:

```text
data/calibration/ftp-sensor-001.csv
data/calibration/ftp-sensor-001.md
```

---

# 35. Sensor Zero Strategy

At minimum, support a user zero.

Example:

1. Engine off.
2. Pressure port equalized to atmosphere.
3. Press and hold ZERO.
4. Record current ADC as `zero_adc`.
5. Display `0.0 inH2O`.

Optional behavior:

- save zero in EEPROM
- provide reset-to-factory calibration
- distinguish between temporary zero and permanent calibration

Do not automatically zero while the engine is running.

---

# 36. Electrical Robustness

This is automotive electrical environment, not a bench project.

Important considerations:

- switched 12 V input
- fuse close to source
- reverse-polarity protection if practical
- TVS transient protection if practical
- stable 5 V converter
- proper grounding
- avoid routing sensor signal near ignition/high-current wiring
- use common sensor/ADC reference
- strain relief on connectors
- vibration-resistant mounting
- enclosure ventilation vs sealing tradeoff
- avoid exposing electronics directly to engine-bay heat if not needed

The selected 8–60 V converter greatly improves input-voltage tolerance but should not be assumed equivalent to OEM load-dump-qualified electronics.

---

# 37. Mechanical Gauge Enclosure

A custom 3D-printed gauge enclosure is likely.

CAD location:

`cad/gauge/`

Potential design goals:

- compact
- clean automotive appearance
- easy visibility from driver seat
- serviceable
- USB or programming access
- secure PCB mounting
- button access
- cable strain relief
- potentially mount in one of the currently unused interior openings

Material should be selected for interior automotive heat.

ASA is a likely enclosure material.

PAHT-CF may be unnecessary for the cabin enclosure unless a structural reason exists.

---

# 38. Potential Holley Integration — Later

The dedicated gauge is currently the priority.

The GM FTP signal may later also be connected to a spare Holley Terminator X 0–5 V input for logging.

Potential benefit:

Correlate crankcase pressure with:

- RPM
- MAP
- TPS
- AFR
- engine load

This is useful for tuning.

However, the current Holley 3.5 in handheld display may not support convenient live custom-channel display.

Therefore the dedicated gauge remains useful even if Holley logging is added later.

Any split/shared analog signal design should be evaluated before implementation.

Do not simply tie multiple analog inputs together without considering input impedance, grounding, and reference voltage.

---

# 39. Current Status

Mechanical:

- Catch can installed in temporary vented mode on September 13 in an earlier project
- Catch-can bracket fabricated and installed during that earlier work; manifold-vacuum PCV remains incomplete
- Initial restrictor CAD completed
- Initial restrictor printed in QIDI PAHT-CF
- 3 mm baseline selected
- 2 mm and 4 mm variants being prepared / printed
- Relief valve ordered
- Relief cracking pressure research performed
- Relief valve may be modified with a lighter spring

Instrumentation:

- Gauge architecture selected at a high level
- GM FTP sensor concept selected
- inexpensive FTP sensor candidate selected
- compatible connector identified
- wide-input 5 V converter ordered
- exact microcontroller/display not yet finalized
- gauge firmware not yet started
- gauge enclosure not yet designed

Repository:

- GitHub repo created
- repo name: `ls-pcv-system`
- initial directory layout planned
- README content planned
- initial commit not yet fully assembled

---

# 40. Near-Term Work Items

Recommended next steps in roughly this order:

1. Finalize root repository files.
2. Add current FreeCAD restrictor source.
3. Add restrictor README.
4. Create BOM.
5. Document current catch-can installation.
6. Photograph catch-can system and restrictors.
7. Receive FTP sensor and connector.
8. Verify sensor pinout.
9. Receive 8–60 V to 5 V converter.
10. Select microcontroller.
11. Select OLED/display.
12. Breadboard FTP sensor.
13. Build simple serial pressure reader.
14. Build water manometer.
15. Characterize actual sensor calibration.
16. Implement zero and min/max.
17. Build in-cabin gauge prototype.
18. Design gauge enclosure.
19. Install pressure line/sensor.
20. Begin 2 / 3 / 4 mm restrictor testing.
21. Tune relief-valve spring/cracking pressure.
22. Record final installation and tuning results.

---

# 41. Questions Still Open

## Mechanical

- Exact final PCV hose routing
- Final catch-can port assignments
- Exact relief-valve cracking pressure after modification
- Whether 3 mm ultimately remains the best restrictor
- Whether intermediate 2.5 / 3.5 mm restrictors are needed
- Whether oil carryover changes with different restrictor sizes
- Exact crankcase-pressure target once real data is available

## Gauge

- Arduino Nano vs another MCU
- OLED size/model
- display layout
- button count
- permanent vs temporary calibration storage
- enclosure location
- whether to add logging
- whether to also feed signal into Holley

## Electrical

- exact sensor pinout
- final input TVS choice
- final fuse size
- exact grounding point
- whether additional RC filtering is required
- whether signal conditioning beyond MCU ADC is needed

---

# 42. Important Constraints for Future Codex Work

When implementing firmware or documentation:

1. Do not invent dimensions that conflict with this file.
2. Treat 3 mm as the current baseline, not a proven final value.
3. Keep tuning data separate from assumptions.
4. Preserve units explicitly.
5. Use `inH2O` as the primary crankcase-pressure unit.
6. Keep raw/calibrated data when practical.
7. Avoid hiding fast pressure spikes with excessive filtering.
8. Do not assume aftermarket sensor wire colors.
9. Do not assume the relief valve is acceptable at 0.5 PSI without modification/testing.
10. Do not assume the FTP sensor transfer curve is exact; calibration is part of the design.
11. Native FreeCAD files are authoritative CAD source.
12. Do not delete STL/STEP exports merely because they are generated if they are intentionally released artifacts.
13. Keep the repository readable as an engineering project, not just source code.
14. Prefer small, well-documented modules over one large monolithic firmware file.
15. Document hardware assumptions in firmware comments or hardware docs where relevant.

---

# 43. Suggested Firmware Directory — Once MCU Is Chosen

Do not create this exact layout until the platform is finalized, but a likely structure is:

```text
firmware/
├── README.md
├── platformio.ini          # if PlatformIO is chosen
├── src/
│   ├── main.cpp
│   ├── pressure_sensor.cpp
│   ├── pressure_sensor.h
│   ├── display.cpp
│   ├── display.h
│   ├── calibration.cpp
│   ├── calibration.h
│   ├── buttons.cpp
│   └── buttons.h
├── include/
├── test/
└── docs/
```

Potential logical modules:

- pressure sensor
- calibration
- filtering
- peak capture
- display
- input/buttons
- persistence
- diagnostics

Keep hardware abstraction simple enough that the sensor/display can be changed later.

---

# 44. Potential Firmware Data Model

Possible internal state:

```text
PressureReading
- rawAdc
- volts
- pressureInH2O
- filteredPressureInH2O
- timestamp

Calibration
- zeroAdc
- slope
- intercept
- valid

PressureStats
- minPressure
- maxPressure
- lastReset
```

Do not over-engineer this until hardware is chosen.

---

# 45. Naming Conventions

Suggested repository naming style:

- lowercase
- kebab-case for directories/files where appropriate
- descriptive names
- units in filenames only when useful
- avoid `final`, `final2`, `latest`, etc.

Examples:

```text
pcv-restrictor-3mm.FCStd
pcv-restrictor-3mm.stl
pcv-restrictor-2mm.stl
pcv-restrictor-4mm.stl
ftp-sensor-calibration.csv
catch-can-installation.md
pressure-test-2026-10-xx.csv
```

For CAD variants, decide whether to use:

- one parametric FreeCAD file with configurable dimension
- separate FreeCAD files per restrictor size
- one source file plus separate STL exports

A parametric single source file is preferable if practical.

---

# 46. Versioning / Releases

Eventually, GitHub releases may be useful.

Potential release artifacts:

- tested STL restrictors
- STEP exports
- gauge enclosure STL
- firmware binary
- wiring diagram PDF/image
- BOM snapshot
- calibration instructions

Do not create formal releases until the parts have been validated.

---

# 47. Safety / Engineering Notes

This project modifies crankcase ventilation on a performance engine.

Incorrect restriction can cause:

- excessive crankcase vacuum
- insufficient crankcase evacuation
- positive crankcase pressure
- oil leaks
- seal stress
- oil carryover
- intake oil contamination

Therefore:

- tune based on actual pressure measurement
- inspect for oil leaks
- inspect catch can contents
- inspect hose integrity
- verify relief path
- do not rely on a single unverified sensor reading
- validate sensor zero periodically
- initially test conservatively

The pressure gauge is a tuning/diagnostic instrument, not a certified safety device.

---

# 48. Initial Codex Objective

If Codex is being initialized on this repository for the first time, recommended first task:

1. Inspect repository contents.
2. Preserve any existing files.
3. Create missing initial directories only when useful.
4. Add or improve root `README.md`.
5. Add `.gitignore`.
6. Add `hardware/bom/parts.md`.
7. Add `docs/testing/test-plan.md`.
8. Add `firmware/README.md` describing the planned gauge without committing to a microcontroller platform prematurely.
9. Do not generate firmware yet unless hardware selection is explicitly requested.
10. Keep this file as historical project context unless asked to split it into more formal docs.

---

# 49. Project Philosophy

The most important goal is not simply to make the system work.

The repository should capture enough engineering context that someone can understand:

- what problem existed
- how the system was changed
- why each component was selected
- why dimensions were chosen
- how pressure was measured
- how calibration was performed
- what the test data showed
- why the final configuration was selected

The repo should function as:

- build documentation
- engineering notebook
- CAD source archive
- electrical design archive
- firmware project
- test-data archive
- future maintenance reference

---

# 50. Current Baseline Snapshot

As of the current design state:

```text
Vehicle:
2002 Pontiac Trans Am WS6
408 CID Gen III LS-based stroker
Naturally aspirated
Holley Terminator X

PCV:
Catch can + manifold vacuum
Interchangeable fixed restrictor
Separate low-pressure atmospheric relief

Restrictor:
Baseline ID:             3.0 mm
Straight throat length:  3.0 mm
Main passage ID:         6.4 mm
Barb bore ID:            6.4 mm
Body OD:                 14.0 mm
Barb OD:                 9.8 mm
Barb crest OD:           10.9 mm
Tip OD:                  ~8.716 mm
Expansion 3 -> 6.4:      8.0 mm
Contraction 6.4 -> 3:    8.0 mm
Overall length:          67.0 mm
Straight root length:    15.0 mm each end
External body taper:     3.0 mm each end
Design status:           Adopted prototype; physical validation pending
Material:                QIDI PAHT-CF

Restrictor variants:
2 mm
3 mm
4 mm

Relief valve:
EVIL ENERGY spring-loaded check valve
Published crack pressure: 0.5 PSI
Desired crack pressure:    ~0.1 PSI
Likely modification: lighter spring

Pressure sensor:
GM 16238399-style FTP sensor
5 V, 3-wire, analog
Low differential-pressure range
Custom calibration planned

Connector:
GM 13585316 / PT2782-compatible pigtail

Power:
SSLHONG B09NVG35CX
8–60 V input
5 V regulated output
3 A max
Ordered

Gauge:
Dedicated in-cabin digital gauge
Microcontroller + OLED concept
Live inH2O
min/max capture
zero/calibration
hardware not fully finalized
```

---

# 51. Reminder to Future Codex Sessions

Before making assumptions, read:

- root README
- this context file
- `cad/pcv-restrictor/README.md`
- BOM
- testing docs
- calibration docs
- any recent logs

If real measured data conflicts with an early design assumption in this file, measured data wins.

When that happens, update the formal project docs rather than silently changing behavior.
