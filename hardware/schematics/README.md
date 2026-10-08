# Schematics and physical layouts

Follow the [standard](../../docs/diagramming-and-wiring-standard.md). Use [Wokwi](../../simulation/wokwi/README.md) for instructional electronics component-and-wire views and temporary solderless breadboard illustrations. L01's complete intended bench connection view lives there; it does not reproduce a permanent protoboard's solder-hole map or actual module footprints.

Create `drawio/` with the first selected installation/spatial illustration that other tools do not express well; use editable `.drawio.svg` with embedded diagram data. Detailed permanent perfboard mapping may use DIY Layout Creator if needed; it is not a current dependency.

A physical-layout illustration is not an electrical schematic or dimensional CAD authority. Circuit schematics, if needed, belong in a named circuit subdirectory with native KiCad files and reviewed exports. Analog conditioning or protection complexity may justify a schematic even without a custom PCB. No detailed carrier layout or circuit schematic has been created yet; see the [inventory](../DIAGRAMS.md).
