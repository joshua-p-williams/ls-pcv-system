# PCV restrictor

Status: initial model and PAHT-CF print reported in the [historical context](../../docs/LS_PCV_SYSTEM_CONTEXT.md); source and exports have not yet been imported. These dimensions describe the agreed design baseline, not measured as-built geometry.

## Baseline geometry

All dimensions below are in mm. Approximate values and ranges remain intentional.

| Feature | Baseline |
| --- | ---: |
| Metering ID / straight length | 3.0 / 3.0 |
| Maximum center body OD | 14.0 |
| Main passage ID | 8.0 |
| Contraction, 8 to 3 ID | 6-8 long |
| Expansion, 3 to 8 ID | 15 long |
| Internal transition, 8 to 6 ID | 4-5 long |
| Barb bore ID | 6.0 |
| Barb root OD | 9.8 |
| Retention crest OD | approximately 10.5 |
| Tip OD / tip taper length | approximately 8.8-9.0 / 3-4 |
| Hose engagement | approximately 16-18; target 17 |
| Clamp land | approximately 8-10 |
| External taper, 14 to 9.8 OD | approximately 7-8 long |

Target hose ID is 3/8 in (9.525 mm). Compare 2.0, 3.0, and 4.0 mm metering variants; 2.5 and 3.5 mm may follow if measurements justify them.

## Design intent

The short metering section localizes restriction. The asymmetric contraction/expansion avoids an abrupt downstream expansion without requiring symmetric internals. A 6 mm barb bore leaves nominal radial wall thickness `(9.8 - 6.0) / 2 = 1.9 mm`, compared with 0.9 mm for an 8 mm bore. These are geometric calculations, not a strength or flow validation.

The baseline material is QIDI PAHT-CF. Reported print orientation is vertical with the fitting axis along Z; keep support out of the passage. Inspect passages, hose-entry edges, and layer integrity. Record actual bore measurements and any finishing operation; a nominal filename does not establish the finished orifice diameter.

## Source and exports

Add the existing native model under `source/pcv-restrictor.FCStd`; do not create a placeholder CAD file. A single parametric source is preferred if practical. Native FreeCAD geometry is authoritative; explain discrepancies with this table before changing either.

Place intentional exports under `exports/stl/` and `exports/step/`. For each export batch, record source revision, FreeCAD version, variant dimensions, units, tessellation settings where applicable, and verification performed. Do not label exports as validated releases until physical testing supports that claim.

Compare configurations using the [test plan](../../docs/testing/test-plan.md).
