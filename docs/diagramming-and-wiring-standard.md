# Diagramming and wiring standard

This standard defines formats, locations, evidence labels and review practices for system diagrams, interfaces and wiring. It favors maintainable text sources and keeps conceptual architecture distinct from physical connections.

## Choose the simplest useful format

| Purpose | Authoritative format | Use |
| --- | --- | --- |
| Requirements, interfaces, pinouts, rationale | Markdown tables and prose | Default when topology adds no value |
| System, pneumatic, power, signal or data flow | Mermaid embedded in Markdown | Relationships and conceptual boundaries, not exact harness wiring |
| Firmware modes, sequencing, test decisions | Mermaid state, sequence or flowchart in Markdown | Choose the diagram type that answers the question |
| Exact connector-to-connector wiring | WireViz `.yml` | Pin mapping, cables, splices, shields, wire color/gauge/length |
| Electronics component-and-wire illustration; temporary solderless breadboard view | Wokwi `diagram.json` | Recognizable components and colored terminal-to-terminal leads, synchronized with the harness |
| Virtual ESP32 firmware testing | Wokwi JSON and build configuration | Document synthetic inputs, model coverage and differences from physical hardware |
| Other installation or spatial illustration | Editable `.drawio.svg` | When other formats do not express placement well |
| Detailed permanent perfboard layout | DIY Layout Creator, if needed | Optional future hole/copper map; no current tooling dependency |
| Custom circuitry, electrical-rule checking or PCB | KiCad native project | Introduce when circuit complexity warrants it; no PCB is required to justify a real circuit schematic |

Markdown owns explanatory context; Mermaid owns conceptual relationships; pinout pages own terminal identity/evidence; WireViz owns designed point-to-point connectivity; the existing BOM owns purchases. Wokwi provides a synchronized instructional view and virtual model, not a competing wiring authority. A generated harness BOM describes build requirements, not purchased inventory. Native CAD remains authoritative for dimensions. See the [Wokwi adoption decision](wokwi-adoption.md).

## Locations and naming

| Location | Responsibility |
| --- | --- |
| `hardware/components/<component>/` | Component descriptions, design notes and import records; indexed in `hardware/components/README.md` |
| `hardware/DIAGRAMS.md` | Central proposed/artifact inventory and selection queue |
| `hardware/architecture/` | System, PCV, power and measurement architecture in Markdown/Mermaid |
| `hardware/pinouts/` | Verified or explicitly unresolved module/connector interface tables |
| `hardware/wiring/bench/` | Development harness source and accompanying build notes |
| `hardware/wiring/vehicle/` | Separate vehicle harness source and installation notes |
| `hardware/wiring/<configuration>/generated/` | WireViz renderings next to their owning source |
| `simulation/wokwi/` | Maintained Wokwi illustration/simulation JSON, physical mappings, model limits and opening instructions |
| `hardware/schematics/drawio/` | Editable layout/installation illustrations when needed |
| `hardware/schematics/<circuit>/` | Future circuit schematic projects, if justified |
| `firmware/` | Firmware architecture/state/data-flow documentation near future code |
| `docs/testing/` | Test/calibration workflow diagrams near procedures |
| Existing `media/` folders | Photos, vendor references and CAD views; no duplicate harness renders |

Use lowercase kebab-case basenames; retain the requested `DIAGRAMS.md` index name. Create artifact and generated subdirectories with their first useful content. Keep sources and outputs on the same basename. Prefer Git history to filenames such as `final-v2`; record formal build revisions inside documents when needed. Do not move existing component notes or replace `hardware/bom/parts.md`.

Keep the `hardware/` root for shared navigation/index documents. Component-specific folders belong under `hardware/components/`; shared architecture, pinout, wiring, schematic, BOM and datasheet records stay in their purpose-specific directories. Moving a component document does not move its CAD or media.

## Writing style

Explain the design for readers unfamiliar with its development history. Lead with purpose, current choices and remaining work. Use direct technical prose such as "The sensor and ADC share the regulated 5 V branch" and keep compatibility checks beside that statement.

Use Git history for routine editing and selection chronology. Dates belong where they identify an installation, test, calibration, import or source revision. Keep provenance in dedicated records or sections. A short scope/status statement is sufficient for a conceptual design; repeat uncertainty only where it changes how a specific detail should be used.

## Required context

Each artifact's Markdown page or adjacent README records: purpose, scope/configuration (concept, bench or vehicle), document state, authoritative source, evidence links, assumptions/TBDs, related inventory ID and applicable hardware revision. Generated artifacts also record source revision/hash, tool versions and exact command. A review applies only to the documented configuration. Use Git history for routine drafting and selection dates; retain dates for test, calibration, installation and source records.

Document states: **Proposed**, **Selected**, **Draft**, **Reviewed**, **Superseded**. Record blockers separately. Reviewed means reviewed documentation, not tested hardware.

Evidence labels on facts/connections: **Planned**, **Manufacturer Spec**, **Vendor Listing**, **Measured**, **Bench Verified**, **Vehicle Verified**, or **Unknown**. Include dated evidence and method for measurements/tests; one tested connection does not validate an entire harness. Distinguish designed from as-built wiring.

## Diagram and interface rules

- Give every diagram a descriptive title, scope and legend. Use labels and line styles as well as color; keep text legible in normal repository previews and printed copies.
- In architecture, distinguish fluid, power and signal paths, label directions and voltage domains, and mark future features/TBD choices. Concept diagrams do not imply terminal mapping or safe assembly instructions.
- Use simple Mermaid syntax compatible with the target preview; visually review the rendered result. Do not create parallel editable Mermaid and Draw.io versions of the same diagram.
- Pinout pages identify exact board/module revision, connector part/marking, cavity numbers, mating versus wire-entry view, latch/key orientation, and source photograph/drawing. Distinguish chip pins, module header labels, board aliases and MCU GPIO numbers.
- Pinout tables record terminal, label/function, power/logic domain, direction, evidence/status and references. A destination may link to the harness; do not maintain a second independent connection table.
- Assign stable, unique connector/cable/net identifiers within each harness, and explicitly map identifiers shared by subassemblies. Reuse them in pinouts, renders and test records. Do not invent unverified physical pin numbers to complete a drawing.
- Wiring notes specify endpoint connector/pin, wire identity, gauge with units, color, length with units, cable/shield grouping, splices, termination and grounding as applicable. Mark unknowns explicitly in accompanying notes. Keep unresolved connections out of assembly-ready renders.
- Never infer FTP pin function from aftermarket wire colors. Use the purchased sensor/pigtail record; the proposal's PT2782 designation is not an established identity for this purchase.
- Show the sensor conditioning boundary; do not turn a conceptual sensor-to-ADC line into a direct connection. Treat protection, rail capacity, USB/backfeed handling, GPIO assignments, and display power as unresolved until documented.
- Keep peak acquisition separate from display smoothing; no engine-running automatic zero. Bench and vehicle configurations are separate artifacts, not interchangeable revisions.

## Sources, outputs and review

WireViz YAML is the editable connectivity source. Resolve errors against evidence, correct the source and regenerate; never manually repair only the output. Commit SVG as the default browsable render with each selected harness. HTML and TSV are optional when useful; PNG/PDF are derivatives only when needed. Keep generated files trackable, and update them with their sources.

Specify planned conductor colors in the harness source and show them in renders and connection schedules. Start with [W01's color convention](../hardware/wiring/bench/generated/connections.md#wire-color-convention) for new project wiring: black ground, red 5 V, orange 3.3 V and labeled signal colors. Keep unresolved color choices explicit. Distinguish designed wire colors from observed factory leads; color never establishes an unknown terminal function. Labels remain authoritative, and as-built records capture substitutions. A whole factory-cable symbol does not specify its internal conductor colors.

Editable Draw.io SVG must include embedded diagram data. Reopen it to verify editability and check its repository preview. If editable SVG is unsuitable, keep `.drawio` source plus a clearly labeled SVG derivative. See the [official editable-image guidance](https://www.drawio.com/docs/manual/collaboration/diagram-data-image-formats/).

Wokwi JSON is the maintained component-and-wire illustration source. Use stable interface IDs, explicit voltage-domain labels and W01 conductor colors; preserve actual GPIO decisions. Record virtual-to-physical terminal mappings, substitutions and omitted connections beside it. Confirm types/pins against official Wokwi definitions, check JSON syntax and unique IDs, and inspect the rendered diagram. Keep unrelated movement/routing out of electrical edits. Update W01 first when changing actual wiring, then synchronize affected Wokwi views and scenarios. Use one diagram for illustration and simulation when practical; explain differences if separate configurations are needed.

Graphical representations and functional models are separate. Clearly mark unsupported or purely illustrative components, and review community model sources/licenses before adoption. Simulated measurements are synthetic; describe whether a fault scenario exercises application logic, a peripheral driver or a modeled bus. Simulation does not validate the RC filter, power paths, translator voltage levels, current capacity or sensor calibration. The selected solderable protoboard construction remains in effect; a Wokwi arrangement is not a dimensional solder-hole map.

Review source and rendered output for clipped text, swapped terminals, orientation ambiguity, units, evidence labels, missing connections, public metadata and private paths. Rendering proves syntax/presentation, not electrical correctness. Physical verification needs a linked test record. Keep source edits, generated changes and inventory state consistent.

## Tooling

Mermaid lives in Markdown; no Node toolchain is required just to author these pages. Use GitHub rendering and an available Mermaid-capable editor preview. Add an editor extension only if needed; no personal editor settings are required.

Use the [Wokwi guide](../simulation/wokwi/README.md) for online/VS Code opening instructions, model status and current licensing limitations. The initial diagram can be inspected without firmware. Add `wokwi.toml` against actual build artifacts when firmware exists; keep compiled outputs and credentials local. Wokwi remains optional for ordinary firmware builds. Add custom chips/scenarios only with useful content; do not add a second component or pinout inventory.

For harness work, use WireViz plus Graphviz (`dot` executable on PATH). Prefer an isolated pipx installation of WireViz; Graphviz is a separate prerequisite. Follow [WireViz installation/usage](https://github.com/wireviz/WireViz) and the installed `wireviz --help`; output options have changed between releases, as documented in [release notes](https://github.com/wireviz/WireViz/releases).

W01 rendering is validated with Python 3.12.10, WireViz 0.4.1 and Graphviz 16.1.0. Use its [pinned requirements and reproduction commands](../hardware/wiring/bench/README.md#rendering-and-validation); the local venv is an alternative to pipx. The [renderer](../hardware/wiring/bench/render.py) generates the complete harness, sectional views and connection schedule from one YAML source. Mermaid preview commands and tested versions are recorded in the architecture pages. For harness changes:

1. Install Graphviz separately and use an isolated Python environment with the harness's pinned requirements.
2. Check `dot -V` and `wireviz --help`; record actual versions.
3. Verify the installed output-directory/format flags and render the selected YAML into its adjacent `generated/` directory.
4. Record the working command and pin the tested WireViz version in repository tooling before adding repeatable automation. Record Graphviz/Python versions as well.
5. Visually inspect the result, then commit selected outputs with their source.

Do not claim rendering passed until a real artifact has been rendered. Regenerate W01 section views and the connection schedule with its renderer after connectivity changes. Add CI or optional graphical tools only when a selected artifact needs them.

## Selection workflow

Use the [inventory](../hardware/DIAGRAMS.md) to identify scope and dependencies. Document architecture, verify module interfaces, design wiring, render/review, and record bench results. A selected conceptual diagram can show TBDs; production pin mapping must not be guessed. Promote a tested bench design into a separately reviewed vehicle harness when installation details are known.
