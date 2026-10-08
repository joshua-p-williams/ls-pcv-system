# W01 connection schedule

Generated from [bench-harness.yml](../bench-harness.yml); do not edit this table.
**Draft: not assembly-ready.** `TBD` names are functional requirements, not terminal IDs.
`W_USB` represents a complete USB cable, not one conductor. Other wire numbers identify
conductors within a documentation bundle, not connector cavities. Colors specify added bench wiring,
not purchased pigtail colors or verified terminal functions. Label and record substitutions.

| Bundle / wire | Net or function | Planned color | Endpoint A | Endpoint B | Evidence / dependency |
| --- | --- | --- | --- | --- | --- |
| W_USB / 1 | USB cable | Unassigned (see note) | USB_SOURCE.PLUG | J1.USB_NATIVE | Complete USB cable, NOT one conductor.; Native USB: left front view / USB label; COM unused.; Computer cable must support data; source/load capacity Q12.; Color unspecified: complete factory cable. |
| W_RAIL / 1 | 3V3 | OG / Orange | J1.L1 | RAILS.3V3 | Carrier feeds: L1 3V3, L22 GND, L21 nominal 5 V.; IN-OUT closed; USB-OTG open; family assumption Q30.; 5 V is after USB input diode; voltage/load checks Q05/Q12.; Length (mm), termination and substitutions: as-built record. |
| W_RAIL / 2 | GND | BK / Black | J1.L22 | RAILS.GND | Carrier feeds: L1 3V3, L22 GND, L21 nominal 5 V.; IN-OUT closed; USB-OTG open; family assumption Q30.; 5 V is after USB input diode; voltage/load checks Q05/Q12.; Length (mm), termination and substitutions: as-built record. |
| W_RAIL / 3 | 5V_USB | RD / Red | J1.L21 | RAILS.5V | Carrier feeds: L1 3V3, L22 GND, L21 nominal 5 V.; IN-OUT closed; USB-OTG open; family assumption Q30.; 5 V is after USB input diode; voltage/load checks Q05/Q12.; Length (mm), termination and substitutions: as-built record. |
| W_LCD / 1 | DISP_RST | WH / White | J1.L5 | J2.RST | Proposed signal allocation Q14.; Verify display logic domain Q06.; Length (mm), termination and color substitutions: as-built record. |
| W_LCD / 2 | DISP_CS | BU / Blue | J1.L16 | J2.CS | Proposed signal allocation Q14.; Verify display logic domain Q06.; Length (mm), termination and color substitutions: as-built record. |
| W_LCD / 3 | DISP_DC | VT / Violet | J1.L4 | J2.DC | Proposed signal allocation Q14.; Verify display logic domain Q06.; Length (mm), termination and color substitutions: as-built record. |
| W_LCD / 4 | SPI_MOSI | GN / Green | J1.L17 | J2.SDA | Proposed signal allocation Q14.; Verify display logic domain Q06.; Length (mm), termination and color substitutions: as-built record. |
| W_LCD / 5 | SPI_CLK | YE / Yellow | J1.L18 | J2.SCL | Proposed signal allocation Q14.; Verify display logic domain Q06.; Length (mm), termination and color substitutions: as-built record. |
| W_LCD_PWR / 1 | DISP_3V3 | OG / Orange | RAILS.3V3 | J2.VCC | Selected 3.3 V display power Q31; listing range 3-5 V.; Current/backlight and regulator margin checks Q06/Q12.; Length (mm), termination and substitutions: as-built record. |
| W_LCD_PWR / 2 | GND | BK / Black | RAILS.GND | J2.GND | Selected 3.3 V display power Q31; listing range 3-5 V.; Current/backlight and regulator margin checks Q06/Q12.; Length (mm), termination and substitutions: as-built record. |
| W_ADC / 1 | 5V_SHARED | RD / Red | RAILS.5V | J3.V | Shared 5 V ADC power and LS1 HV1/HV2 bus.; Verify Q07/Q08/Q12; translator bring-up Q29.; Length (mm), termination and substitutions: as-built record. |
| W_ADC / 2 | GND | BK / Black | RAILS.GND | J3.G | Shared 5 V ADC power and LS1 HV1/HV2 bus.; Verify Q07/Q08/Q12; translator bring-up Q29.; Length (mm), termination and substitutions: as-built record. |
| W_ADC / 3 | I2C_SDA_HV | GN / Green | LS1.HV1 | J3.SDA | Shared 5 V ADC power and LS1 HV1/HV2 bus.; Verify Q07/Q08/Q12; translator bring-up Q29.; Length (mm), termination and substitutions: as-built record. |
| W_ADC / 4 | I2C_SCL_HV | YE / Yellow | LS1.HV2 | J3.SCL | Shared 5 V ADC power and LS1 HV1/HV2 bus.; Verify Q07/Q08/Q12; translator bring-up Q29.; Length (mm), termination and substitutions: as-built record. |
| W_ADDR / 1 | ADDR_GND | BK / Black | RAILS.GND | J3.ADDR | PROPOSED address 0x48.; Inspect existing bias before fitting Q08/Q18.; Length (mm), termination and color substitutions: as-built record. |
| W_BUTTON / 1 | BUTTON | GY / Gray | J1.L6 | S1.CONTACT_A | Normally open; internal pull-up proposed.; GPIO6 candidate Q14; wire to real contact pair.; Length (mm), termination and color substitutions: as-built record. |
| W_BUTTON / 2 | GND | BK / Black | RAILS.GND | S1.CONTACT_B | Normally open; internal pull-up proposed.; GPIO6 candidate Q14; wire to real contact pair.; Length (mm), termination and color substitutions: as-built record. |
| W_FTP_PWR / 1 | SENSOR_5V | RD / Red | RAILS.5V | FTP.C | P04: GM-family map assumed; C=5 V, A=return.; USB-fed L21 with IN-OUT closed; voltage/load checks Q05/Q12.; Q09: check lead mapping/fit at assembly.; Colors apply to added leads, NOT factory pigtail.; Length (mm), termination and substitutions: as-built record. |
| W_FTP_PWR / 2 | SENSOR_RETURN | BK / Black | RAILS.GND | FTP.A | P04: GM-family map assumed; C=5 V, A=return.; USB-fed L21 with IN-OUT closed; voltage/load checks Q05/Q12.; Q09: check lead mapping/fit at assembly.; Colors apply to added leads, NOT factory pigtail.; Length (mm), termination and substitutions: as-built record. |
| W_RAW / 1 | SENSOR_OUTPUT | BN / Brown | FTP.B | COND.IN | FTP B signal to RC filter IN.; R1/C1 circuit in rc-input-filter component guide.; BN applies to added lead, NOT factory pigtail.; Length (mm), termination and substitutions: as-built record. |
| W_A0 / 1 | FILTERED_A0 | PK / Pink | COND.OUT | J3.A0 | RC OUT (R1/C1 junction) to A0.; Keep short; input stays within ADC GND to VDD.; Power/protection review Q17.; Length (mm), termination and substitutions: as-built record. |
| W_ANALOG_GND / 1 | ANALOG_RETURN | BK / Black | RAILS.GND | COND.GND | C1 return to common analog reference.; No display-only return path.; Length (mm), termination and substitutions: as-built record. |
| W_I2C_LV / 1 | I2C_SDA_LV | GN / Green | J1.L12 | LS1.LV1 | ESP32 3.3 V I2C to LS1 LV1/LV2.; Selected channels 1=SDA, 2=SCL.; Pull-up/communication bring-up Q29. |
| W_I2C_LV / 2 | I2C_SCL_LV | YE / Yellow | J1.L15 | LS1.LV2 | ESP32 3.3 V I2C to LS1 LV1/LV2.; Selected channels 1=SDA, 2=SCL.; Pull-up/communication bring-up Q29. |
| W_LS_REF / 1 | LS_3V3_REF | OG / Orange | RAILS.3V3 | LS1.LV | Selected module LV/HV supplies and both GND pads.; 5 V shares the sensor/ADC branch.; No enable pin; bring-up Q29; selected source Q30; load checks Q12. |
| W_LS_REF / 2 | LS_5V_REF | RD / Red | RAILS.5V | LS1.HV | Selected module LV/HV supplies and both GND pads.; 5 V shares the sensor/ADC branch.; No enable pin; bring-up Q29; selected source Q30; load checks Q12. |
| W_LS_REF / 3 | LS_GND_LV | BK / Black | RAILS.GND | LS1.GND_LV | Selected module LV/HV supplies and both GND pads.; 5 V shares the sensor/ADC branch.; No enable pin; bring-up Q29; selected source Q30; load checks Q12. |
| W_LS_REF / 4 | LS_GND_HV | BK / Black | RAILS.GND | LS1.GND_HV | Selected module LV/HV supplies and both GND pads.; 5 V shares the sensor/ADC branch.; No enable pin; bring-up Q29; selected source Q30; load checks Q12. |

## Wire color convention

Project convention for W01 additions, not a universal electrical color standard.
Use labels to distinguish buses sharing clock/data colors. Color alone does not identify a net.
Display VCC uses orange for the selected 3.3 V supply; the nominal 5 V branch uses red.
The factory USB cable has no specified jacket or internal-conductor color.
Blank-color lines render neutrally; they are not instructions to use ground wire.

| Code | Color | Use |
| --- | --- | --- |
| BK | Black | Ground/return, including ADDR-to-GND strap |
| RD | Red | Nominal USB-fed 5 V supply, shared sensor/ADC branch and translator HV reference; Q30 selected, voltage/load checks Q12 |
| OG | Orange | 3.3 V supply |
| YE | Yellow | Clock: SPI CLK or I2C SCL, identified by bundle/net |
| GN | Green | Data: SPI MOSI or I2C SDA, identified by bundle/net |
| BU | Blue | Display chip select |
| VT | Violet | Display data/command |
| WH | White | Display reset |
| GY | Gray | Button signal |
| BN | Brown | Raw sensor output, added harness lead only |
| PK | Pink | Conditioned ADC A0 signal |

## Proposed controller assignments

Derived from J1 in the same source. L coordinates refer to P01's front-view left row.
These are design proposals under Q14, not measured pin functions.

| J1 location | GPIO / function |
| --- | --- |
| USB_NATIVE | Native USB / left |
| L1 | 3V3 |
| L4 | GPIO4 / DISP_DC |
| L5 | GPIO5 / DISP_RST |
| L6 | GPIO6 / BUTTON |
| L12 | GPIO8 / I2C_SDA |
| L15 | GPIO9 / I2C_SCL |
| L16 | GPIO10 / DISP_CS |
| L17 | GPIO11 / SPI_MOSI |
| L18 | GPIO12 / SPI_CLK |
| L21 | 5Vin / USB-fed 5 V |
| L22 | GND |
