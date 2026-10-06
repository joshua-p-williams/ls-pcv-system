# Project status

## Initial repository baseline - 2026-10-05

Imported documentation from the [historical context](LS_PCV_SYSTEM_CONTEXT.md). Physical progress below is reported there, not independently verified in this repository.

- No catch can in the original September 6 planning state; a temporary vented can and bracket were installed September 13 in an earlier project. See the [imported installation record](installation/2026-09-13-catch-can.md). Manifold-vacuum PCV remains incomplete.
- Initial restrictor modeled and printed in QIDI PAHT-CF, as reported in the context.
- 3 mm baseline selected; 2 and 4 mm variants being prepared/printed.
- Relief valve and power converter ordered.
- FTP sensor candidate and compatible connector family identified.
- MCU/display selection, firmware, calibration, and enclosure remain pending.
- Five sanitized historical installation photographs and a dated installation record have been imported. No CAD source, schematics, or measured logs have been imported yet.

## Next work

- [ ] Import native restrictor CAD and intentional exports; document variant/export provenance.
- [ ] Confirm BOM quantities, costs, procurement status, and exact purchased parts.
- [x] Import the earlier catch-can installation history and bracket photos.
- [ ] Verify and document current hose routing, fresh-air path, and catch-can port assignments.
- [ ] Select MCU/display and document voltage compatibility and ADC reference strategy.
- [ ] Verify actual sensor connector terminals before powering hardware.
- [ ] Characterize sensor zero, slope, repeatability, and usable range.
- [ ] Measure relief opening/reseating behavior and reverse leakage.
- [ ] Implement and bench-validate pressure reading, zero, and min/max capture.
- [ ] Record repeatable restrictor comparisons using the test plan.

## Open decisions

Final hose routing; relief spring and verified setting; final restrictor diameter; pressure limits justified by measurements; MCU/display; buttons; calibration persistence; fuse/transient protection; grounding; sensor mounting; gauge enclosure; optional logging/Holley integration; repository license.

For each resolved decision, record the date, rationale, evidence link, and affected configuration in the relevant component document. Keep this page focused on current status rather than session transcripts.
