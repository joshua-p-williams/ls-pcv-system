# Hardware documentation

Start with the [diagram inventory](DIAGRAMS.md) to choose upcoming documentation and the [diagramming/wiring standard](../docs/diagramming-and-wiring-standard.md) for formats, responsibilities, evidence labels and tooling.

- [Components](components/README.md): component descriptions, design notes and import records.
- [Architecture](architecture/README.md): system, PCV, power and measurement relationships. Current drafts: [A04: engine PCV overview](architecture/pcv-system.md), [A01: overall gauge architecture](architecture/gauge-system.md), [A02: power architecture](architecture/power-system.md), and [A03: measurement signal chain](architecture/measurement-system.md).
- [Pinouts](pinouts/README.md): module and connector identities and interface evidence.
- [Wiring](wiring/README.md): separate bench and vehicle harnesses.
- [Schematics and layouts](schematics/README.md): optional spatial illustrations and future circuit projects.
- [BOM](bom/parts.md): existing procurement authority.
- [Datasheets](datasheets/README.md): preserved reference revisions.

Use the [gauge integration guide](components/gauge-electronics/README.md#reading-and-building-path) for the learning and assembly path. Separate component guides cover the [N16R8 development board](components/esp32-s3-dev-board/README.md), [XIAO](components/xiao-esp32s3/README.md), [GC9A01 display](components/gc9a01-display/README.md), [ADC](components/adc/README.md), [sensor/pigtail](components/fuel-tank-pressure-sensor/README.md), [power supply](components/gauge-power-supply/README.md), and [relief valve](components/check-valve-relief-valve/README.md). Interface pages link to these explanations; the BOM retains procurement responsibility.

## Organization

Keep this directory as the entry point for hardware documentation. Only shared navigation and inventory documents live directly here; group substantive content by purpose in the directories above. New component records belong in `components/<component>/` and are listed in the [component index](components/README.md).

Architecture, pinouts, harnesses, schematics, procurement and datasheets remain shared categories alongside `components/`. Reuse those categories before introducing a new one. Native CAD and media retain their existing repository locations; component pages connect the relevant records through links.
