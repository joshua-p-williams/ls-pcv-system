# Harness documentation

Use WireViz YAML for exact physical connectivity and adjacent Markdown for scope, interfaces, evidence and reproduction commands. Follow the [standard](../../docs/diagramming-and-wiring-standard.md) and select work from the [inventory](../DIAGRAMS.md).

Track unresolved scope, power and interface dependencies in the [open-questions register](../../docs/open-questions.md). W01 scope and power choices remain open; an incremental controller/display build has not yet been selected.

Create `bench/` for development wiring and `vehicle/` for installed wiring with their first selected artifact. Each configuration owns its YAML and a `generated/` directory for SVG and optional HTML/TSV outputs. No harness YAML or connection assignments have been created yet.

Start with one bench harness. Sensor, display and ADC subassemblies are inventory candidates; split them into separate YAML only if they are physically separable and independently useful. Otherwise document them within the bench source to avoid conflicting maps. Vehicle wiring requires its own power, grounding, routing and service decisions.

Generated harness BOMs describe construction requirements; [parts.md](../bom/parts.md) remains the procurement record. Record tool versions, source revision/hash and the verified render command alongside each harness. Never hand-edit generated connectivity.
