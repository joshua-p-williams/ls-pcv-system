# Harness documentation

Use WireViz YAML for exact physical connectivity and adjacent Markdown for scope, interfaces, evidence and reproduction commands. Follow the [standard](../../docs/diagramming-and-wiring-standard.md) and select work from the [inventory](../DIAGRAMS.md).

Track unresolved power and interface dependencies in the [open-questions register](../../docs/open-questions.md). Q01 establishes the complete bench-design scope below and Q02 selects the N16R8 controller; Q03 selects alternate USB sources with a restart between modes. Power-path verification and pin assignments remain open.

The [W01 bench harness](bench/README.md) contains the complete design draft, proposed GPIO assignments, generated connection schedule and phased assembly views. `bench/` owns its YAML and adjacent `generated/` outputs. Add `vehicle/` with its first selected artifact; installed wiring remains a separate configuration.

Start with one bench harness. Sensor, display and ADC subassemblies are inventory candidates; split them into separate YAML only if they are physically separable and independently useful. Otherwise document them within the bench source to avoid conflicting maps. Vehicle wiring requires its own power, grounding, routing and service decisions.

Generated harness BOMs describe construction requirements; [parts.md](../bom/parts.md) remains the procurement record. Record tool versions, source revision/hash and the verified render command alongside each harness. Never hand-edit generated connectivity.

## W01 scope and assembly

W01 documents the complete intended bench wiring: controller, display, ADC, sensor, input conditioning, power distribution and one multifunction button. Its [assembly guide](bench/README.md#phased-assembly-and-verification) derives section views from the full source. Resolve the marked gaps and verify endpoint assignments before treating it as assembly-ready. Phased assembly does not reduce the overall design scope.

During drafting, mark unresolved terminals, component values or connections inline as TBD with their open-question IDs and a link to the register. Do not draw a guessed connection as established wiring. Resolve the applicable design and interface questions before issuing instructions to assemble or energize that section; measured performance remains a separate validation activity.

Include an assembly section describing the selected build order and verification gates. Each phase should identify the connections to assemble, required power/disconnection state, checks and expected results, and the conditions for proceeding. Smaller wiring views may highlight the relevant portion of the complete design, using the same connector, pin and net identifiers. Derive these views from the authoritative harness source rather than maintaining independent connectivity definitions. Identify any temporary test connections explicitly.

W01 proposes the sequence and verification gates; unresolved section prerequisites must be completed before use. Preserve measured results separately and update the main design if testing requires changes. Q01 selects this documentation approach; it does not resolve the remaining terminal, power or component questions.

## Controller configurations

W01 uses the N16R8 ESP32-S3 development board. The XIAO ESP32-S3 remains the finished-gauge target and requires the same complete wiring design, explicit pin assignments and phased assembly/verification approach. Bench results inform the XIAO design but do not verify its different power paths, GPIO allocation or load capacity.

Keep each board's physical connections explicit in its harness configuration, with shared functional names where useful. Match those assignments to the corresponding [firmware board configuration](../../firmware/README.md#board-configurations). A XIAO bench validation configuration and installed vehicle wiring have distinct operating conditions; do not substitute the N16R8 map or treat a board change as only a firmware change. Create the XIAO artifacts when their scope is selected, retaining W05 for vehicle wiring.

## Bench power requirements

The N16R8 bench setup must preserve USB programming/debugging and also operate without a computer. Powering the complete system from the computer's USB connection is desirable if the verified board paths and load budget support it; external power remains an option. Firmware must start and perform its gauge functions without waiting indefinitely for a USB host or serial console.

The selected W01 arrangement uses one USB power source at a time: computer USB for power and development, or a suitable standalone USB supply for operation without a computer. Unplugging and restarting when changing sources is acceptable. The [power plan](../components/esp32-s3-dev-board/power-plan.md) selects native USB, IN-OUT closed, USB-OTG open and L21 nominal 5 V distribution (Q30). Display VCC uses board 3V3 (Q31). Actual rail voltage, regulator margin and source/cable capacity remain normal bring-up checks (Q05/Q12). The standalone supply is not yet selected. A converter-fed USB source would additionally require Q11 verification.

Simultaneous computer and external power, and uninterrupted source switching, are outside the selected W01 mode. Q19 remains open for later converter/vehicle integration. If Q05/Q12 show the selected USB arrangement cannot support the complete load, revisit the implementation explicitly; do not silently add another supply. Neither the two pictured USB ports nor a generic ESP32-S3 schematic proves the purchased board supports simultaneous sources.

## Bench construction and retention

W01 must form a sturdy assembly that can be moved without intermittent contacts or repeated reconnection. Loose jumper pins inserted into solderless breadboards are not the intended final bench construction. Temporary probing fixtures may be used during checks, but must not define the completed harness.

The selected construction is a solderable protoboard/perfboard carrier with female header strips soldered to it. Modules use male pin headers and plug into those sockets for removal and replacement. Permanent carrier wiring and component connections are soldered. The available sockets are female header strips, not solderless breadboard contacts. Protoboard, male headers and female socket strips are reported on hand in existing inventory. Use nominal 2.54 mm (0.1 inch) pitch as the accepted planning assumption; no photos or product identification are required to continue design. Check module row spacing, clearance, contact fit and retention during layout/assembly.

Identify the actual carrier pad/track pattern before laying out connections: isolated pads and linked strips require different wiring and isolation. Do not assume a solderable board has the same internal connections as a solderless breadboard. Record any required cuts or bridges in the assembly documentation. Carrier size and pad pattern remain layout details; nominal 2.54 mm pitch is assumed. These checks do not block drafting the electrical connectivity.

Secure modules mechanically and provide strain relief so cable movement does not pull on contacts or solder joints. Choose a common support or carrier and retain cables/connectors where needed; do not rely on solder alone as the mechanical support for loose modules. Record connector orientation, wire identity and retention in W01, and check continuity and operation after normal handling of the assembled bench unit.

Q16 is resolved as a construction policy. Use a convenient mixture of soldered leads and secure removable connections at assembler discretion, including off-board display, button and sensor wiring. Do not mandate a connector family or exact mechanical arrangement unless an electrical or fit requirement makes it necessary.

W01 must specify endpoint connectivity, polarity, return routing and applicable wire/current/voltage requirements. Follow its [wire color convention](bench/generated/connections.md#wire-color-convention) for added wiring; label both ends and record substitutions when stock differs. The assembler may choose compatible terminations, practical lengths and mechanical placement within those constraints. Record the actual connectors, their pin orientation, wire identity/length and any departures in the build record or as-built harness. A chosen connector pin order must be mapped explicitly before use; it is not implied by a generic drawing.

Electrical wire sizing and return routing remain harness design work, not unanswered user preferences. Routine construction choices do not require separate consultation. The completed assembly still needs firm retention, mechanical support and strain relief. Vehicle mounting and vibration requirements remain a separate W05 design.
