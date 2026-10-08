# N16R8 bench-board reference evidence

The supplied references support a YD-style 44-position ESP32-S3 development-board family as the candidate for W01. They do not establish the identity or electrical behavior of the received unit. [P01](../../pinouts/esp32-s3-devkit.md) remains the interface map; Q04 and Q05 remain Investigating in the [question register](../../../docs/open-questions.md).

## Identification and source limits

The staged notes associate Amazon B0D93DLB6Q with YEJMKJ branding and a YD/VCC-GND board family. The rear product image carries YEJMKJ branding and visibly reads `YD-ESP32-23` and `2022-V1.3`; retain that transcription rather than silently replacing it with the family's commonly used YD-ESP32-S3 name. These markings identify the pictured reference, not a confirmed received revision.

The images are product illustrations, including loose header strips, promotional captions and third-party branding. All supplied images were collected online; none depicts the project's physical board. The purchased N16R8 board has not yet arrived, so physical identification and electrical checks await receipt. The front image shows ESP32-S3-WROOM-1 and N16R8 markings. Its 22 labels per side agree with P01's existing row order, including `5Vin` near the bottom left. The colored pin chart simplifies that label to 5V; it does not prove this pin supplies USB-derived power.

| Imported reference | What it contributes |
| --- | --- |
| [Front product view](../../../media/reference/gauge-electronics/bench-board/board-front-reference.png) | Module marking, header labels and front jumpers |
| [Rear product view](../../../media/reference/gauge-electronics/bench-board/board-back-reference.png) | Pictured board/revision marking, COM/USB labels and USB-OTG jumper |
| [Hardware introduction](../../../media/reference/gauge-electronics/bench-board/hardware-introduction.png) | Annotated USB roles, CH343P, regulator and LED locations |
| [Component callouts](../../../media/reference/gauge-electronics/bench-board/component-callouts.png) | Second annotated reference for ports and indicators |
| [Header reference](../../../media/reference/gauge-electronics/bench-board/header-reference.png) | Visual corroboration of the existing 44-position label map |

## USB and power findings

With the antenna at the top and connectors at the bottom, component side facing the viewer, the annotations identify **left: native ESP32-S3 USB; right: CH343P USB-to-UART**. The rear view reverses left/right and labels the ports USB and COM. W01 now selects the native port for power, programming and native USB debugging; the COM port stays disconnected.

The [official board-family V1.4 schematic](https://github.com/vcc-gnd/YD-ESP32-S3/blob/main/5-public-YD-ESP32-S3-Hardware%20info/YD-ESP32-S3-SCH-V1.4.pdf) establishes the reference circuit: native VBUS enters the internal 5V rail through D2, header pin 21 enters through D3, and IN-OUT bypasses D3. W01 selects IN-OUT closed for a USB-fed header output, with USB-OTG open. In that schematic USB-OTG bypasses D2; the earlier community description of tying both raw VBUS rails together is incomplete because the UART input diode remains. The [power plan](power-plan.md) explains connections, diode drop and routine bring-up.

The rear listing image reads 2022-V1.3; the official reference is V1.4. Its circuit is adopted as a working family assumption, not a confirmed received revision. The images show the relevant jumper locations and matching header map. No additional photos are required to continue design. Q04/Q05 retain received-board comparison and ordinary electrical checks; Q12 retains actual source/load margin. No physical jumper change or measurement is recorded.

Q03's single-source USB choice still stands. Q30 resolves the USB-fed distribution/debug arrangement. No simultaneous USB sources, external header supply or converter/host combination is selected.

## Module reference and pin choices

The preserved [Espressif module datasheet v1.8](../../datasheets/esp32-s3-wroom-1_wroom-1u-datasheet-v1.8.pdf) identifies N16R8 as 16 MB Quad-SPI flash and 8 MB Octal-SPI PSRAM (Table 1-1). It describes the module, not this carrier's regulator, USB connectors or power jumpers. [Datasheet provenance](../../datasheets/README.md#esp32-s3-wroom-1-and-wroom-1u) records the revision and checksum.

The staged notes propose GPIO8/9 for I2C and discuss direct MCU analog inputs. These are not adopted assignments: Q14 still owns the pin plan, and the project uses the external ADS1115 for pressure acquisition. Preserve the existing memory, boot and USB restrictions in P01. Reserve GPIO48 provisionally while the RGB connection is checked.

## Import record - 2026-10-06

Batch: `bench-board-verification`. Reviewed all seven supplied files: five product/reference images, one research README and one Espressif PDF. Public technical findings are summarized here rather than copying the conversational README, unsupported conclusions, tracking links or proposed alternative folder tree.

- `front.jpg` and `back.jpg` became `board-front-reference.png` and `board-back-reference.png`.
- The AVIF hardware-introduction image became `hardware-introduction.png`.
- The long-name JPEG ending in `cores (1).jpg` became `component-callouts.png`; the one ending in `cores.jpg` became `header-reference.png`.
- Image copies contain newly encoded PNG pixels with no embedded metadata. Verified identical decoded RGB pixels and dimensions, with no orientation tags in the originals. Visible product markings/branding were retained; no personal details requiring visual redaction were observed.
- The supplied module PDF was copied unchanged after checking document metadata, attachment absence and link-only annotations. Cover rendered and visually reviewed; variant table and revision history inspected. It is not a carrier schematic.
- Inbox originals remain unchanged. No physical inspection, continuity/power test, memory detection, jumper change or firmware execution was performed.
