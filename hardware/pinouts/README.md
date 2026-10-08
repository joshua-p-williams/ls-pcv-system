# Module and connector interfaces

Create a module page when selected from the [inventory](../DIAGRAMS.md). Follow the [standard](../../docs/diagramming-and-wiring-standard.md); each page identifies its source evidence and outstanding physical checks.

Each page records exact module/revision, connector identity, viewing orientation and evidence, followed by a table with terminal/cavity, printed label, function, voltage domain, direction, evidence state and source. Explicitly distinguish chip pins from module headers and GPIO aliases. Put unresolved details in a short verification checklist. Link to the authoritative harness for destinations.

Initial candidates are the N16R8 bench board, XIAO ESP32-S3, ADS1115 module, GC9A01 display, purchased FTP sensor/pigtail, and SSLHONG converter. Verify actual variants; generic chip documentation does not prove a module's header order. Optional controls follow selection.

[P01: ESP32-S3 N16R8 bench board](esp32-s3-devkit.md) provides an image-based header map, GPIO restrictions and pending interface assignments. Physical board verification remains open.

[P03: ADS1115 module](ads1115.md) documents the image-based header order, TI pin functions, address options, 5 V ADC supply and translated 3.3 V controller interface; physical module verification and configuration remain open.

[P02: XIAO ESP32-S3](xiao-esp32s3.md) documents the manufacturer edge-pin map, aliases, power references and allocation constraints for the finished-gauge controller.

[P04: FTP sensor and pigtail](ftp-sensor.md) records connector-view conventions, unresolved terminal assignments, planned electrical/pressure interfaces and the evidence needed before wiring.

[P05: GC9A01 display](gc9a01.md) documents the pictured SPI header, controller signal budget and selected 3.3 V supply/logic and remaining current/backlight checks.

[P06: SSLHONG converter](buck-converter.md) records labeled input polarity, the USB-C power-output boundary and checks for power distribution, returns and USB coexistence.
