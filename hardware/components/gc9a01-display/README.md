# GC9A01 round TFT display

## What the display contributes

The first gauge prototype uses a round **TFT** (thin-film-transistor LCD) to show signed live pressure and min/max values in inH2O. The **GC9A01** is the display-controller designation; the purchased module also includes the panel, PCB, connector and backlight arrangement. A controller name alone does not specify the complete module's power circuitry or mechanical outline.

The ESP32 sends commands and pixel data to the display. The display presents the result; it does not measure pressure or apply sensor calibration. [A03](../../architecture/measurement-system.md) keeps acquisition and peak capture independent of screen refresh and display smoothing, so a readable update rate need not discard a brief pressure event.

### Understanding SPI and control signals

**SPI** is a serial interface with clock and data signals. The clock times the transfer; data carries commands or pixel information. Chip select (**CS**) identifies the receiving device, data/command (**DC**) identifies the kind of transfer, and reset (**RST**) restarts the display controller.

The purchased module calls its clock and data pins `SCL` and `SDA`. Those labels do not imply I2C: this module's documented interface is SPI. Do not connect them to the ADC bus. The supplied image shows no MISO/readback pin, so the planned interface is for display writes. Use [P05](../../pinouts/gc9a01.md) for header order, direction and electrical constraints rather than copying a generic display wiring example.

## Selected hardware and evidence

Purchased item: **Hosyond 1.28-inch round GC9A01 TFT**, [Amazon B0DYP4J9XP](https://www.amazon.com/dp/B0DYP4J9XP). **Three displays were purchased for USD 14.39 total**. Advertised resolution is **240 x 240 pixels**, with a 4-wire SPI description. Receipt and operation remain unconfirmed; see the [BOM](../../bom/parts.md).

![Vendor GC9A01 display image](../../../media/reference/gauge-electronics/gc9a01-display.jpg)

The notes describe a square PCB and approximately 32.4 mm active circular area. The supplied image instead shows a rounded PCB with a connector tab and mounting holes. Use the actual module's measured outline, mounting holes, connector clearance, and viewing area for enclosure design; the approximate active-area figure is not an enclosure dimension.

![Vendor display pin reference](../../../media/reference/gauge-electronics/gc9a01-pin-definition.jpg)

P05 owns the terminal map. The [original electronics import record](../gauge-electronics/README.md#import-record---2026-10-06) preserves provenance and image limitations. These are listing references, not photographs of a tested assembly.

## Electrical integration and readable output

The [purchased listing](https://www.amazon.com/dp/B0DYP4J9XP) specifies a **3-5 V operating supply** and ESP32 compatibility. The design selects **3.3 V VCC and 3.3 V SPI/control signals** (Q31), directly from the controller's 3V3 output. This voltage lies within the advertised module range and keeps the display in the controller's logic domain without another translator. It does not establish 5 V signal tolerance or the exact regulator/backlight circuit. Q06 retains current/backlight behavior and Q12 retains regulator margin.

Module supply voltage and signal voltage are different specifications. A module advertised for 5 V power may regulate that power internally without accepting 5 V on its data pins. The project uses 3.3 V for both display power and signals; the ADC's 5 V domain remains a separate translated I2C interface.

A circular viewing area sits within the pixel coordinate space. Text or indicators near the corners of a square layout can therefore be clipped by the visible circle. Check rotation, signed values, units, min/max labels, invalid/stale indication and readability from the driver's seat before finalizing the enclosure or choosing a larger screen.

Brightness is fixed initially, with software configuration and later persistent settings planned where supported. No dimming GPIO is selected, and this module's backlight-control capability remains unresolved. A software option must not imply hardware capability that has not been established.

Follow the display section of [W01's assembly guide](../../wiring/bench/README.md#phase-2-display-and-button). A working test image checks initialization, orientation and basic communication; it does not establish full-gauge sampling timing or vehicle readability. Q06/Q20 in the [question register](../../../docs/open-questions.md) track supply and bring-up work.

## Display options retained for later

- One 1.28-inch pressure gauge: initial prototype and possible final configuration.
- Several small displays in a pod: potential future channels include fuel/oil pressure, oil/coolant temperature, or differential pressure. Shared SPI clock/data with separate chip selects is a proposed approach, pending pin-budget and bus/driver validation.
- A larger approximately 2.1-inch round display if readability requires it. No specific larger module is selected or purchased; active-screen size does not establish compatibility with a conventional gauge opening.

These are future options, not requirements to implement multiple channels now. Keep display size and controller choice out of pressure-measurement logic.

