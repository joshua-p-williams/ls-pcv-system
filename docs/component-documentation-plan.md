# Component documentation improvement plan

The repository combines an engineering record with explanations that help readers understand and reproduce the project. Component READMEs introduce the underlying concepts; interface pages, harness sources, the BOM and test records retain responsibility for precise implementation details.

## Instructional approach

Each component page should answer the following questions, using only the sections and examples that are useful for that part:

1. What is it, and what does it do in this system?
2. How does it work, and which terms does a new reader need?
3. Why is it used here, and what tradeoff does that choice make?
4. How does it connect to the surrounding components?
5. What can an assembly check or experiment demonstrate, and what remains unknown?

Explain concepts before specifications and import history. Keep a distinction between hypothetical examples, selected design values, vendor claims and measured results. Link to the authoritative interface, harness or procedure instead of copying its tables. Explanations should follow the current shared-5 V ADC architecture and use inH2O for project pressure examples.

## Adopted electronics organization

The electronics use separate component folders, with a shared integration guide:

| Destination under `hardware/components/` | Responsibility |
| --- | --- |
| `esp32-s3-dev-board/` | N16R8 bench controller, chip/module/carrier distinctions, memory, USB and board-specific evidence |
| `xiao-esp32s3/` | Compact finished-gauge controller, aliases/pin budget and board power constraints |
| `gc9a01-display/` | TFT/display-controller concepts, SPI and display-control signals, layout/readability and module limitations |
| `gauge-electronics/` | Short integration guide explaining how the controller, ADC, translator, display and button work together; retain the original multi-component import record |

The bench-board evidence page is stored with its controller. Keep existing media paths stable; update relative links, plain-text references and any section anchors affected by the split. P01/P02/P05 remain the authoritative interface maps, and the BOM remains the procurement record. The sensor/pigtail stays together because it forms one useful interface assembly.

## Work sequence and progress

- [x] Establish the instructional purpose in the repository entry point, component index and AGENTS.md.
- [x] Add an initial explanation of sensor/calibration, converter/power and check-valve/relief concepts alongside the existing technical records.
- [x] Add an electronics introduction distinguishing acquisition, computation and display responsibilities.
- [x] Separate N16R8, XIAO and GC9A01 component pages; move bench-board evidence to N16R8, retain the shared integration/import guide and update references.
- [x] Review the ADC introduction for input-interface consistency, resolution versus accuracy and explicit hypothetical examples in inH2O, preserving its educational intent.
- [x] Introduce the restrictor's flow role, bore-area comparison and distinction from the catch can using the existing CAD record.
- [x] Connect the ADC introduction to the separate controller/display pages.
- [ ] Extend the catch-can/PCV explanation using the existing installation and architecture records.
- [x] Add a reading path through concepts, interfaces, complete wiring, phased assembly and calibration in the [gauge integration guide](../hardware/components/gauge-electronics/README.md#reading-and-building-path).

For each completed change, check links and filename case, current design consistency and whitespace. Render and inspect any changed diagrams. Documentation review does not establish physical performance or replace calibration and bench results.
