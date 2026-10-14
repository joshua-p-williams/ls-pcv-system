# Bench protoboard arrangement

For the reported carrier, use [L04’s solder-side layout and illustrated schedule](protoboard/README.md). This page remains general construction guidance; L04 supplies the specific pad coordinates and reproducible drawings.

**Draft placement proposal.** This plan turns [W01's electrical connections](README.md) into a sturdy, serviceable bench assembly. It uses the on-hand nominal 2.54 mm solderable protoboard, male headers and female socket strips. Module dimensions, carrier size and pad pattern do not need to be documented exhaustively to plan the arrangement; check that they fit during a dry placement before soldering.

The [L01 Wokwi view](../../../simulation/wokwi/README.md) and [PNG](../../../simulation/wokwi/diagram-preview.png) explain the connections. Their virtual module footprints and positions are illustrative. The sketch below proposes physical zones, viewed from the component side; it is not to scale and contains no connector pin-order or solder-pad instructions. W01 remains the wiring authority.

## Placement sketch

The [L03 breadboard view](../../../simulation/wokwi/breadboard/README.md) and [PNG](../../../simulation/wokwi/breadboard/diagram-preview.png) provide a carrier-based illustration: controller near the centre, ADC/filter and translator together, sensor to the left and display to the right. Its rails/strip connections are explicit; virtual module footprints still need translation to the actual solderable carrier. The zone sketch below remains a flexible alternative arrangement, not a second required layout.

```text
                 Display on its own supported mount
                      short cable to controller

     +--------------------------------------------------+
     | Controller zone          | Digital interface     |
 USB | J1 N16R8 in sockets      | LS1 level shifter      |
edge | USB plugs accessible    | near J1 and J3         |
     | antenna faces free space|                        |
     |-------------------------+------------------------|
     | Supply distribution     | Analog measurement     |
     | distinct 3V3 / 5V / GND  | J3 ADS1115 in sockets   |
     | supported cable exits   | R1/C1 directly near A0  |
     +--------------------------------------------------+
           |                           |
     Supported button           Sensor cable entry
                                FTP mounted separately
```

This is a starting arrangement, not a required compass orientation. Rotate or shift the zones to suit the carrier while preserving access and short routes. Do not infer that the controller's native USB socket is on a particular side of this sketch: identify it using [P01 and the power plan](../../components/esp32-s3-dev-board/power-plan.md), then face it toward an accessible carrier edge.

| Zone | Placement and reason |
| --- | --- |
| J1 controller | Socketed near an edge; leave room for USB plugs, reset/boot controls and removal. Keep its antenna end clear of wiring bundles and metal supports where practical |
| LS1 translator | Between the controller and ADC so both I2C segments can stay short. Identify LV/HV from the real module labels; Wokwi symbol order is not a footprint |
| J3 ADC and COND filter | Put R1/C1 beside A0. The R1 output, C1 signal lead and A0 form a compact node; C1's return goes to the nearby ADC ground reference |
| Supply distribution | Label separate 3V3, 5V and common GND connection points. These can be supported soldered branches; no terminal-block purchase is required |
| Display and button | Mount where readable/reachable, with wires restrained at the carrier and the off-board part. Leave only enough slack to service them |
| FTP sensor | Support separately so the hose and pigtail cannot load the carrier. Route its signal toward the ADC/filter zone rather than through the display wiring |

## Wire routing and gauge defaults

Use insulated copper hook-up wire. **22 AWG for the main supply/return feed and display power, and 24 AWG for the remaining short bench connections** are proposed starting defaults. They are not a claim that those gauges are already in inventory or that the power budget is verified. Compatible stock can be used after checking termination fit and loaded rail voltage. W01's gauge fields remain TBD until the actual wire choice is recorded; these defaults guide that choice.

| W01 bundles | Proposed starting gauge | Routing |
| --- | --- | --- |
| W_RAIL | 22 AWG | Short controller-to-distribution feeds, including common return |
| W_LCD_PWR | 22 AWG | Display supply and return together; separate branch from the analog section |
| W_LCD / W_BUTTON | 24 AWG | Keep display clock/data/control together; keep button signal with its return |
| W_ADC / W_I2C_LV / W_LS_REF / W_ADDR | 24 AWG | Short carrier routes with correctly separated 3.3 V and 5 V I2C segments; no unused channel wires |
| W_FTP_PWR / W_RAW / W_A0 / W_ANALOG_GND | 24 AWG | Sensor signal near its return; keep OUT-to-A0 and C1 return especially short. Preserve the purchased pigtail rather than replacing its factory wire gauge |
| W_USB | Factory USB data cable | Secure the cable independently; do not fabricate a USB harness |

AWG is American Wire Gauge; a lower number means a thicker conductor. Thickness affects resistance and termination fit, while the insulation and construction affect flexibility and durability. Stranded wire is preferred for leads that move. Solid wire is convenient for fixed carrier jumpers if supported against repeated bending. Do not force an oversized wire or several strands into a socket intended for a module pin.

The gauge defaults are project choices, not manufacturer prescriptions. For scale, [Alpha Wire's 22 AWG 3051](https://www.alphawire.com/en/products/wire/hook-up-wire/premium/3051) and [24 AWG 3050](https://www.alphawire.com/products/wire/hook-up-wire/premium/3050) document stranded copper hook-up constructions. Those references are examples, not selected purchases or ratings for unidentified stock. Final load margin remains Q12; thicker branch wire cannot correct an overloaded board regulator or USB source.

There is **no required cut-length schedule**. Lay out each route and cut to fit, allowing a small service bend and enough reach for module removal without pulling a solder joint. Avoid large loops or coils of spare wire. Exact lengths are optional build details unless a long route, voltage drop or bus problem makes them relevant. If a route requires a substantially longer cable than this compact bench arrangement, review its power drop or interface behavior before treating it as equivalent.

Keep these routing priorities:

- Keep the filtered A0 node and its local return compact. Do not bundle that node with SPI clock or display power wiring.
- Give display and analog loads their own return branches to common ground; the display return must not be the analog section's sole path back to the controller.
- Run each supply with its return where practical. All returns remain electrically common; no isolated analog ground or chassis bond is introduced.
- Keep I2C on the carrier and SPI display leads short. A wire's color does not distinguish the two buses; label the bundle or net at both ends.
- Follow [W01's colors](generated/connections.md#wire-color-convention). Record and label stock-color substitutions. Assign FTP terminals from P04, never factory pigtail color.

## Secure connections

Solder female socket strips to the carrier and fit modules with male headers. Align both socket rows using the unpowered module before soldering them fully; remove it for carrier wiring. Support the module mechanically so plugging USB or moving a cable cannot lever it out of its sockets. Keep removal possible without pulling on wires.

Permanent carrier jumpers may be soldered directly. Off-board display, button and sensor leads may use soldered connections or snug removable connectors at assembler discretion. Use locking or otherwise positively retained housings where convenient; a loose individual jumper socket is not adequate retention for a movable assembly. If a removable connection cannot stay secure during ordinary handling, use a better-retained connector or solder the lead.

Anchor cable insulation to the carrier or its support before the solder joint or connector. Leave a small relaxed section between that anchor and the electrical termination. Heat-shrink insulates exposed soldered joins; it does not by itself anchor a cable. Use insulated standoffs or suitable clearances beneath the solder side, and keep metal fasteners away from exposed pads.

Label accessible points **3V3, 5V, GND, COND.IN and COND.OUT/A0** for meter probes. These can be the existing supported connection pads, not additional purchased test sockets. Unused ADC inputs/ALERT and translator channels stay unconnected.

## Build checklist

1. With power disconnected, place the sockets, modules and off-board mounts. Check USB access, module removal and room for a probe; move parts until the arrangement works with the actual carrier.
2. Check the carrier's pad connectivity. Isolated pads need wire bridges; linked strips may need cuts. Record any cuts/bridges clearly, and identify the solder-side view as mirrored relative to this component-side sketch.
3. Fit and solder sockets, then make the labeled supply/return branches. Keep 3V3 and 5V separate. Inspect for bridges before inserting modules.
4. Add the short ADC/filter wiring, translator wiring and controller signal connections from W01. Add supported off-board leads, cut to fit, and label both ends.
5. Check continuity against the [W01 schedule](generated/connections.md), including R1 separating IN/OUT. Do not treat the capacitor's charging indication as a permanent short.
6. Follow [W01's phased assembly and verification](README.md#phased-assembly-and-verification) for energizing and testing. Construction order does not bypass the power, bus and sensor checks. Record measured results separately.
7. Check retention during normal handling with power off first, then confirm operation after handling once bring-up is complete. Record actual wire gauges, connector orientation, stock R1/C1 and any design departures; a photograph or simple annotated sketch can capture the finished routing. Exact wire lengths are optional.

No hole-by-hole map is required for this bench build. If reproducing the exact carrier later becomes useful, use the actual pad grid and footprints for a DIY Layout Creator map as described in the [diagram standard](../../../docs/diagramming-and-wiring-standard.md). Until then, the placement sketch, W01 schedule and build record are sufficient design documentation. No firmware, physical assembly or test results are introduced by this plan.
