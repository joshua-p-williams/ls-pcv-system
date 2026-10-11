# Mechanical designs

Native FreeCAD files define the saved geometry. Mesh exports and slicer projects support fabrication; a saved project does not establish which settings were used for a physical print. Component guides distinguish CAD dimensions, reported fabrication and measured performance.

| Design | Contents and status |
| --- | --- |
| [PCV restrictor](pcv-restrictor/README.md) | Native 3 mm baseline, 2/3/4 mm print variants, model views; PAHT-CF print reported |
| [Pressure tap and hose barbs](pressure-tap/README.md) | Inline pressure-sensing block and separate barbs, native models, STL exports, slicer projects and views; PAHT-CF pressure-tap print reported |

Use `source/` for native CAD, `exports/` for intentional STL/STEP derivatives, and `printing/` for new slicer projects. The pressure tap currently retains its supplied `prints/` folder; links preserve that path pending any agreed naming cleanup. Model views belong under `media/diagrams/<component>/`.

Record source revision, units, export settings when known, and physical print/finishing results in the component guide. Preserve older import records as history. FreeCAD backup files remain local and Git-ignored.
