# Solder-side coordinate record

The 2026-10-13 corrected solder-side map supersedes all earlier mixed-face coordinates. Use printed solder-side labels only. A2 is the repeated A after Z. Pads are individually isolated.

ESP32 left header (project J1.L, named from component front) is column Q, rows 01-22. Its other header is column A2. ADC, display and translator entries follow the supplied full maps. R1/C1, button, bus taps and sensor-landing pads are proposed new placements; check stock-part body fit before soldering. Button contact assignments require the released/pressed continuity check, not an assumed internal pinout. FTP-5V has moved from 18/J to 18/D to clear the button. Former button landing pads 20/M and 20/O are released. Off-board sensor terminals intentionally have no carrier coordinate.

| ID | Component terminal / node | Solder-side row / column |
| --- | --- | --- |
| J1.L1 | ESP32 3V3 (J1.L1) | 01 / Q |
| J1.L2 | ESP32 3V3 (J1.L2) | 02 / Q |
| J1.L3 | ESP32 RST (J1.L3) | 03 / Q |
| J1.L4 | ESP32 GPIO4 (J1.L4) | 04 / Q |
| J1.L5 | ESP32 GPIO5 (J1.L5) | 05 / Q |
| J1.L6 | ESP32 GPIO6 (J1.L6) | 06 / Q |
| J1.L7 | ESP32 GPIO7 (J1.L7) | 07 / Q |
| J1.L8 | ESP32 GPIO15 (J1.L8) | 08 / Q |
| J1.L9 | ESP32 GPIO16 (J1.L9) | 09 / Q |
| J1.L10 | ESP32 GPIO17 (J1.L10) | 10 / Q |
| J1.L11 | ESP32 GPIO18 (J1.L11) | 11 / Q |
| J1.L12 | ESP32 GPIO8 (J1.L12) | 12 / Q |
| J1.L13 | ESP32 GPIO3 (J1.L13) | 13 / Q |
| J1.L14 | ESP32 GPIO46 (J1.L14) | 14 / Q |
| J1.L15 | ESP32 GPIO9 (J1.L15) | 15 / Q |
| J1.L16 | ESP32 GPIO10 (J1.L16) | 16 / Q |
| J1.L17 | ESP32 GPIO11 (J1.L17) | 17 / Q |
| J1.L18 | ESP32 GPIO12 (J1.L18) | 18 / Q |
| J1.L19 | ESP32 GPIO13 (J1.L19) | 19 / Q |
| J1.L20 | ESP32 GPIO14 (J1.L20) | 20 / Q |
| J1.L21 | ESP32 5Vin (J1.L21) | 21 / Q |
| J1.L22 | ESP32 GND (J1.L22) | 22 / Q |
| J1.R1 | ESP32 GND (J1.R1) | 01 / A2 |
| J1.R2 | ESP32 TX (J1.R2) | 02 / A2 |
| J1.R3 | ESP32 RX (J1.R3) | 03 / A2 |
| J1.R4 | ESP32 GPIO1 (J1.R4) | 04 / A2 |
| J1.R5 | ESP32 GPIO2 (J1.R5) | 05 / A2 |
| J1.R6 | ESP32 GPIO42 (J1.R6) | 06 / A2 |
| J1.R7 | ESP32 GPIO41 (J1.R7) | 07 / A2 |
| J1.R8 | ESP32 GPIO40 (J1.R8) | 08 / A2 |
| J1.R9 | ESP32 GPIO39 (J1.R9) | 09 / A2 |
| J1.R10 | ESP32 GPIO38 (J1.R10) | 10 / A2 |
| J1.R11 | ESP32 GPIO37 (J1.R11) | 11 / A2 |
| J1.R12 | ESP32 GPIO36 (J1.R12) | 12 / A2 |
| J1.R13 | ESP32 GPIO35 (J1.R13) | 13 / A2 |
| J1.R14 | ESP32 GPIO0 (J1.R14) | 14 / A2 |
| J1.R15 | ESP32 GPIO45 (J1.R15) | 15 / A2 |
| J1.R16 | ESP32 GPIO48 (J1.R16) | 16 / A2 |
| J1.R17 | ESP32 GPIO47 (J1.R17) | 17 / A2 |
| J1.R18 | ESP32 GPIO21 (J1.R18) | 18 / A2 |
| J1.R19 | ESP32 GPIO20 (J1.R19) | 19 / A2 |
| J1.R20 | ESP32 GPIO19 (J1.R20) | 20 / A2 |
| J1.R21 | ESP32 GND (J1.R21) | 21 / A2 |
| J1.R22 | ESP32 GND (J1.R22) | 22 / A2 |
| J3.V | ADC V | 07 / F |
| J3.G | ADC G | 08 / F |
| J3.SCL | ADC SCL | 09 / F |
| J3.SDA | ADC SDA | 10 / F |
| J3.ADDR | ADC ADDR | 11 / F |
| J3.ALERT | ADC ALERT | 12 / F |
| J3.A0 | ADC A0 | 13 / F |
| J3.A1 | ADC A1 | 14 / F |
| J3.A2 | ADC A2 | 15 / F |
| J3.A3 | ADC A3 | 16 / F |
| J2.RST | Display RST | 01 / I |
| J2.CS | Display CS | 01 / J |
| J2.DC | Display DC | 01 / K |
| J2.SDA | Display SDA | 01 / L |
| J2.SCL | Display SCL | 01 / M |
| J2.GND | Display GND | 01 / N |
| J2.VCC | Display VCC | 01 / O |
| LS1.HV4 | Translator HV4 | 07 / I |
| LS1.HV3 | Translator HV3 | 08 / I |
| LS1.GND_HV | Translator GND_HV | 09 / I |
| LS1.HV | Translator HV | 10 / I |
| LS1.HV2 | Translator HV2 | 11 / I |
| LS1.HV1 | Translator HV1 | 12 / I |
| LS1.LV4 | Translator LV4 | 07 / M |
| LS1.LV3 | Translator LV3 | 08 / M |
| LS1.GND_LV | Translator GND_LV | 09 / M |
| LS1.LV | Translator LV | 10 / M |
| LS1.LV2 | Translator LV2 | 11 / M |
| LS1.LV1 | Translator LV1 | 12 / M |
| R1.IN | R1 sensor-side lead | 16 / H |
| R1.OUT | R1 ADC-side lead | 13 / H |
| C1.OUT | C1 signal-side lead | 13 / J |
| C1.GND | C1 ground-side lead | 15 / J |
| T.GND | FTP-GND landing pad | 18 / F |
| T.SIG | FTP-SIG landing pad | 18 / H |
| T.5V | FTP-5V landing pad | 18 / D |
| S1.CONTACT_A | Button signal leg (check switched pair) | 18 / I |
| S1.CONTACT_B | Button ground leg (check switched pair) | 20 / N |
| S1.LEG_18N | Button extra leg; no added wire | 18 / N |
| S1.LEG_20I | Button extra leg; no added wire | 20 / I |
| FTP.A | FTP pigtail A / ground | Off-board |
| FTP.B | FTP pigtail B / signal | Off-board |
| FTP.C | FTP pigtail C / 5 V | Off-board |
| GH | Ground bus tap | 04 / H |
| GI | Ground bus tap | 04 / I |
| GJ | Ground bus tap | 04 / J |
| GK | Ground bus tap | 04 / K |
| GL | Ground bus tap | 04 / L |
| GM | Ground bus tap | 04 / M |
| GN | Ground bus tap | 04 / N |
| P5H | Nominal 5 V bus tap | 03 / H |
| P5I | Nominal 5 V bus tap | 03 / I |
| P5J | Nominal 5 V bus tap | 03 / J |
| P5K | Nominal 5 V bus tap | 03 / K |
| P3M | 3.3 V bus tap | 03 / M |
| P3N | 3.3 V bus tap | 03 / N |
| P3O | 3.3 V bus tap | 03 / O |
| AGH | Analog ground bus tap | 17 / H |
| AGI | Analog ground bus tap | 17 / I |
| AGJ | Analog ground bus tap | 17 / J |

See [solder plan](solder-plan.md) for every wire, bus link, part value and circuit explanation.
