# Adopt Wokwi for electronics illustrations and firmware simulation

**Status: Adopted.** Wokwi is the preferred tool for instructional component-and-wire views, temporary solderless breadboard illustrations when useful, and virtual ESP32 firmware testing. It provides maintainable JSON connections. [L01](../simulation/wokwi/README.md) covers complete intended bench connectivity using native and original terminal-only parts; functioning system simulation is future work.

## Responsibilities

Markdown explains the design, Mermaid shows architecture and workflows, WireViz defines harness connectivity, and Wokwi illustrates components/connections and supports simulation. Draw.io remains available for installation or spatial illustrations that these tools do not express well. KiCad owns electrical schematics and PCB designs when warranted. DIY Layout Creator may be considered for detailed permanent perfboard layouts; it is not a current dependency.

The real bench uses solderable protoboard with secure connections and removable modules. Adopting Wokwi does not change that construction decision. Its view describes an arrangement and connections, not exact solder holes, copper paths, mounting dimensions or purchased-board geometry.

## Organization and source control

Maintain the initial JSON and adjacent guide under `simulation/wokwi/`, linked from hardware documentation. One diagram can serve illustration and later simulation when the connections agree. Avoid a duplicate `docs/hardware/` tree or copied pinout/BOM records. Add distinct board/configuration diagrams only when needed, with explicit differences.

Commit readable JSON with stable identifiers and minimal unrelated routing changes. W01 remains the authoritative bench connection source; reconcile Wokwi with it whenever electrical interfaces change. Keep virtual-to-physical mappings and model limitations beside the diagram. Existing hardware decisions take precedence over simulation convenience.

## Initial scope and limits

Use the supported ESP32-S3-DevKitC-1 as a documented substitute for the purchased N16R8 carrier, preserving its selected GPIO functions. The full illustration uses original generic terminal-only parts for ADS1115, GC9A01, translator, sensor, distribution and C1. They are not physical footprints or behavior models. Custom-part definitions and inert stubs support the online illustration; VS Code preview additionally requires compiled custom-chip registration, deferred until useful build infrastructure exists. Visual likeness, accurate terminals and simulated behavior are separate capabilities.

No project firmware exists yet. Add `wokwi.toml` against actual compiled artifacts when the planned PlatformIO environments are introduced; do not invent build paths or introduce a separate gauge implementation merely to run the diagram. Keep Wokwi optional for ordinary builds and hardware work.

Future simulation should identify synthetic inputs and test application behavior using shared measurement logic. Fault scenarios must state which layer they exercise. Physical supply behavior, analog conditioning, sensor calibration and assembly retention still require hardware testing.

## Adoption checks

- Standards and agent guidance identify Wokwi's illustration and simulation responsibilities.
- A source-controlled ESP32-S3 diagram and opening instructions exist.
- Drawn GPIO functions and colors agree with W01; physical substitutions are explicit.
- Missing peripheral models and the absence of runnable project firmware are documented.
- Existing Markdown, Mermaid, WireViz and general Draw.io conventions remain available.

See the [diagramming standard](diagramming-and-wiring-standard.md) for ongoing review practices and the [Wokwi guide](../simulation/wokwi/README.md) for tooling sources and validation evidence.
