# USB bench power and debugging

W01 uses one USB source and one cable: a computer for power/programming/debugging, or a standalone USB supply for operation without a computer. Disconnect and restart when changing sources. The following connections are selected from the listing images and published board-family documentation; no received-board measurements are recorded.

## Selected connections

| Function | Selected implementation |
| --- | --- |
| USB port | Left/native ESP32-S3 USB, component side facing the viewer, antenna up and connectors down. Rear views reverse left/right; use the USB label rather than COM |
| USB cable | Factory data-capable cable for computer operation; compatible power cable/source for standalone operation |
| IN-OUT | Closed: fit a solder bridge if open, with all power disconnected. This exposes the internal USB-fed nominal 5 V rail at the 5Vin header |
| USB-OTG | Open for this USB-device setup; no native-port VBUS diode bypass is selected |
| 5 V distribution | P01 left row 21 / 5Vin to carrier 5V, supplying FTP sensor C, ADS1115 V and translator HV together |
| 3.3 V distribution | P01 left row 1 / 3V3 to carrier 3V3, supplying display VCC and translator LV |
| Common return | P01 left row 22 / GND to carrier GND; sensor, ADC, filter, translator, display and button share this reference |
| Unused USB port | COM / CH343P remains disconnected in W01; no second source or cable is required |

The [W01 source and schedule](../../wiring/bench/README.md) own conductor IDs, colors and external connections. These are board-header coordinates, not USB contact numbers. The [display interface](../../pinouts/gc9a01.md) records the vendor's 3-5 V range and selected 3.3 V supply.

## Why IN-OUT needs closing

The [VCC-GND Studio V1.4 schematic](https://github.com/vcc-gnd/YD-ESP32-S3/blob/main/5-public-YD-ESP32-S3-Hardware%20info/YD-ESP32-S3-SCH-V1.4.pdf) shows native USB2 VBUS feeding the internal 5V net through D2. Header J1 pin 21 feeds that net through D3 in the opposite direction for external board power. IN-OUT is in parallel with D3. With IN-OUT open, the header can act as an input but is not a supported USB-derived output. Closing IN-OUT bypasses D3 and connects that header to the internal rail.

The native USB path still includes D2: closing IN-OUT does **not** bypass the USB input diode or produce a separately regulated precision 5.000 V supply. Cable and diode voltage drop depend on load. The net name 5V denotes its nominal domain; measure it at the sensor and ADC during bring-up and confirm it suits the adopted sensor supply requirement (Q10/Q12). The ADS1115's internal reference does not automatically cancel supply-related sensor error.

USB-OTG is a different jumper. In this schematic it bypasses D2 between native USB VBUS and the internal rail. Leave it open for the selected device/debug setup. Describing it simply as tying both raw USB VBUS rails together misses the remaining UART-port diode D1. Do not bridge unrelated USB-JTAG or RGB pads as part of this power change.

The listing's rear image reads 2022-V1.3; the official schematic is V1.4. Adopt this documented family power arrangement as the working assumption, supported by the pictured IN-OUT/USB-OTG pads and header map. This is not a claim of identical received revisions. A different pad layout or failure of the expected rail check is a concrete reason to revisit the assumption; additional board photos are not a design prerequisite.

## USB debugging and standalone behavior

The [board designer's guide](https://github.com/vcc-gnd/YD-ESP32-S3) identifies native USB for programming, communication and JTAG. The ESP32-S3 [USB Serial/JTAG documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-guides/usb-serial-jtag-console.html) describes its serial console and debugger interface. W01 reserves GPIO19/20 for USB. Firmware must select the matching native USB console/debug configuration; a physical cable alone does not configure software or prove debugger operation. No firmware settings are implemented yet.

The right COM port is a separate UART bridge and is not native JTAG. It can support a separately documented recovery/programming session, but it is not the selected W01 source. Disconnect the current source before changing ports. Do not connect both USB ports, inject external 5 V, or add the vehicle converter alongside the computer in this configuration.

A standalone supply uses the same native port and power path, without a USB host. Firmware must start the gauge without waiting indefinitely for host enumeration or a serial terminal. Q19 retains converter/vehicle and simultaneous-power design work; this USB-only selection does not resolve those configurations.

## Current budget and normal bring-up

The designer describes a 1 A 3.3 V regulator; that is a board-family rating, not 1 A of spare peripheral capacity. Controller consumption, regulator heating, source current limits, cable resistance and USB behavior all affect the usable load. The sensor/module/display operating and startup currents are not yet recorded, so Q12 remains open. Do not assume a computer port supplies a standalone charger's advertised current, or that passive USB-C resistors guarantee a negotiated current budget.

As a first budget, USB input current includes the board plus the sensor, ADC, translator and display loads. An LDO (linear regulator) supplies approximately the same current into its 3.3 V loads as it draws from its input, plus its own small operating current. Its approximate dissipated power is `(input voltage - 3.3 V) x 3.3 V load current`; this explains why a regulator current headline is not enough. Establish source limits and measure actual load rather than assigning invented currents.

1. With everything unplugged, check the labeled jumper locations against the reference. Close IN-OUT if necessary; leave USB-OTG open. Check that the bridge is confined to those pads and that power/ground are not shorted.
2. Keep peripherals disconnected, attach one source to native USB, and check 3V3 and 5Vin against GND. Confirm the expected polarity and rail voltage before fitting the distribution wires or peripherals. If the expected nominal 5 V output is absent or unsuitable, stop that phase and revise the power path rather than bridging more pads.
3. Add the display, then ADC/translator, then sensor/filter using [W01's phases](../../wiring/bench/README.md#phased-assembly-and-verification). Record rail voltage, source/current limits, startup/operating current and heating with display activity. Check the sensor and ADC see the same branch and review power-down behavior.
4. Test disconnect/restart with the standalone source. Record source/cable and jumper state with the build. Measurements validate the chosen assembly; they are not prerequisites for documenting this intended wiring.

Q30 resolves this bench power/debug topology. Q05 retains hardware behavior and Q12 retains load margin. No jumper has been physically modified or electrical test performed by this documentation change.
