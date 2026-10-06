# Parts and procurement

Initial inventory transcribed from the [project context](../../docs/LS_PCV_SYSTEM_CONTEXT.md) on 2026-10-05. Status describes what that context reports; it is not a fresh inventory or vendor verification. Published specifications and compatibility claims still require verification on the actual parts. TBD means unknown, not zero or not required.

| Category / part | Manufacturer / identifier | Source | Status | Qty | Cost | Specification / purpose / notes |
| --- | --- | --- | --- | --- | --- | --- |
| Catch can | EVIL ENERGY; product B087LZHFGC reported | Amazon, owner notes | Installed 2026-09-13 in earlier project | 1 | Not published | 300 ml baffled can; temporary vented mode, not completed manifold PCV; see [installation record](../../docs/installation/2026-09-13-catch-can.md) |
| PCV hose | Manufacturer / PN TBD | TBD | In use | TBD | TBD | 1/2 in SAE 30R7 reported; final routing/lengths TBD; restrictor targets 3/8 in ID hose |
| Catch-can bracket | Custom aluminum angle / steel assembly | Fabricated | Installed 2026-09-13 in earlier project | 1 assembly | TBD | Two reported 3.5 mm pop rivets, M10 x 1.5 bolt/washer, Permatex Seal+Lock; inspection after heat cycles pending |
| Restrictor filament | QIDI PAHT-CF | TBD | Used for initial print | TBD | TBD | Record spool and print settings with future builds |
| 3 mm restrictor | Custom | Printed | Initial print complete | TBD | TBD | Baseline; measured bore and vehicle validation pending |
| 2 mm restrictor | Custom | Printed | Preparing / printing | TBD | TBD | Comparison variant |
| 4 mm restrictor | Custom | Printed | Preparing / printing | TBD | TBD | Comparison variant |
| Relief/check valve | EVIL ENERGY; exact SKU TBD | TBD | Ordered | TBD | TBD | Published 0.5 PSI opening; desired approximately 0.1 PSI requires modification/testing |
| Replacement relief spring | TBD | TBD | Under consideration | TBD | TBD | Measure valve/spring and bench-test before selecting |
| FTP pressure sensor | Aftermarket GM 16238399-style; 16196060 / 12219388 cross-references | Amazon candidate B0CNZ2Q1F2 | Candidate selected | TBD | TBD | 5 V, three-wire analog concept; pinout and actual transfer function unverified |
| Sensor pigtail | HiSport candidate; PT2782 / GM 13585316 family; PT2646 referenced | TBD | Identified | TBD | TBD | Verify terminal positions and physical fit; wire colors are not authoritative |
| Power converter | SSLHONG B09NVG35CX | Amazon | Ordered | TBD | TBD | Listed 8-60 V input, 5 V / 3 A USB-C output; not established as OEM load-dump qualified |
| MCU | TBD; Nano-class board is one candidate | TBD | Undecided | TBD | TBD | Platform and ADC/reference compatibility must be selected |
| Display | TBD OLED | TBD | Undecided | TBD | TBD | Size, interface, and voltage compatibility TBD |
| Buttons | TBD | TBD | Planned | TBD | TBD | Zero and peak reset; count/behavior TBD |
| Gauge enclosure / fasteners | Custom; material TBD | TBD | Planned | TBD | TBD | ASA is a candidate; mounting and heat exposure TBD |
| Input fuse / holder | TBD | TBD | Planned | TBD | TBD | Rating and location to be documented in electrical design |
| TVS / input filter | TBD | TBD | Under consideration | TBD | TBD | Select alongside power protection design |
| Wiring / connectors / USB-C breakout | TBD | TBD | Planned | TBD | TBD | Gauge installation and strain relief |

For updates, record exact purchased identifiers, source link, quantity, actual cost/currency, and receipt/installation date when known. Keep candidate alternatives distinct from installed parts. Store verified wiring in `hardware/schematics/` when available; link manufacturer documentation with its revision/access date rather than inventing specifications.
