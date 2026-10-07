# System architecture

Use Markdown with embedded Mermaid for conceptual PCV, gauge, power and measurement relationships. See the [standard](../../docs/diagramming-and-wiring-standard.md) and [inventory](../DIAGRAMS.md).

The [engine PCV overview](pcv-system.md) describes the planned airflow and instrumentation. Each selected page should define its scope, show known subsystem boundaries, distinguish proposed/current states and label unresolved interfaces. Do not use architecture diagrams as pin-to-pin assembly instructions. Firmware-specific diagrams belong near firmware and test workflows near test procedures; index both centrally.

The [overall gauge architecture](gauge-system.md) expands A04's gauge block into measurement, controller, display, control and power boundaries.

The [gauge power architecture](power-system.md) expands A01 into supply paths, return references, operating modes and unresolved power decisions (A02).

The [pressure measurement signal chain](measurement-system.md) details calibration, zero, validity and independent peak/display paths (A03).
