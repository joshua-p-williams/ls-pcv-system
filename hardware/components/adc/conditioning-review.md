# Pressure-input conditioning and logic interface

## Selected electrical architecture

The FTP sensor and ADS1115 share the regulated **5 V branch**. ESP32-S3 logic remains **3.3 V**, with a bidirectional I2C translator between the controller and ADC. Q27 records this selection; it applies to both the N16R8 bench build and the XIAO gauge. The actual bench 5 V takeoff remains Q05/Q12.

The nominal sensor signal reaches A0 without divider scaling. The initial [RC input filter](../rc-input-filter/README.md) uses 470 ohm series resistance and 1 uF from the A0-side node to ground. W01 COND now names its IN/OUT/GND nodes. Q17 retains physical response, power transitions and any additional protection. The advertised sensor family and nominal 5 V analog interface are the working design basis, not a measured transfer curve. Calibrate the assembled chain under Q10/Q21.

The sensor is on hand and unmarked; its pigtail, ADC modules and project converter have not arrived. A multimeter and alternative bench/USB supplies are available, but their suitability depends on the particular test.

## Input range and acquisition

Select **+/-6.144 V** full-scale range for A0 single-ended measurement, giving **187.5 uV per count**. This covers the expected unscaled signal, including a nominal 0-5 V design envelope. The programmable range does **not** permit an input beyond the actual ADC supply: normal analog input voltage must remain between GND and VDD. A nominal 0.5-4.5 V sensor output fits a 5 V-powered ADC; the actual sensor response remains a calibration question.

Removing the divider avoids its attenuation and resistor-ratio error. Sharing the sensor/ADC supply reduces independent-rail mismatch, but does not prove transient behavior or guarantee that every fault is safe. The ADC uses an internal reference; shared supply does not automatically cancel a supply-dependent sensor response.

## I2C translation requirements

At VDD = 5 V, the ADS1115 minimum specified logic-high input is 0.7 VDD = **3.5 V**. Pulling its bus only to 3.3 V therefore does not provide a guaranteed interface. Use bidirectional translation suitable for open-drain I2C, with controller-side pull-ups to 3.3 V and ADC-side pull-ups to 5 V. Account for resistors already fitted to the breakout; do not expose ESP32 pins to 5 V.

Q28 selects the [hiBCTR BSS138 module](../i2c-level-shifter/README.md): LV at 3.3 V, HV at 5 V, common ground, channel 1 SDA and channel 2 SCL. No enable GPIO is required. Initial speed remains a 100 kHz candidate. Actual pull-ups, bus levels, communication and power transitions are bring-up work under Q08/Q29.

W01's LS1 now uses the selected module's visible terminal labels. GND_HV/GND_LV distinguish the two pads both marked GND; they are documentation aliases. Match the component's solder-side orientation reference. LV and HV signal ports must not be directly bridged.

## Filtering and power behavior

The built RC circuit is selected as an initial prototype: R1=470 ohm, C1=1 uF nonpolar, nominal cutoff 339 Hz and time constant 0.47 ms. Its [component guide](../rc-input-filter/README.md) owns internal connectivity, stock-part requirements, calculations and assembly. Earlier divider-based filter values are superseded. ADC supply bypassing is separate; inspect fitted decoupling under Q08. Q17 remains Investigating for stock-part recording, actual response and power/fault behavior; no clamp or isolation stage is selected.

Do not independently power the sensor while the ADC is off. Check local rail differences, startup/shutdown and any retained charge from the selected filter during bring-up. No analog isolation IC or op-amp is selected. Add one only if the resulting circuit or a demonstrated operating condition requires it.

## Superseded proposals

| Earlier proposal | Current decision |
| --- | --- |
| ADS1115 powered from 3V3 | Sensor and ADC share 5 V |
| 12/20 kohm example, then 15/20 kohm divider candidate | No analog divider |
| +/-4.096 V gain for a divided signal | +/-6.144 V for the unscaled signal |
| 100 nF signal capacitor with divider-derived cutoff | Initial 470 ohm / 1 uF RC selected; physical response and power/protection remain Q17 |
| Analog isolation switch / adapter as the next implementation choice | No default analog isolation stage; assess the shared-supply circuit |
| Direct 3.3 V controller-to-ADC I2C | Separate 3.3 V and 5 V segments through a translator |

## Remaining work

- Q29/Q08: account for fitted translator/ADC pull-ups and check bus levels, communication and power behavior during bring-up; Q28 selection is resolved.
- Q05/Q12: establish the USB-derived shared 5 V source and load budget.
- Q17: record stock R1/C1, validate the selected filter and review power transitions/any justified additional protection.
- Q18: settle address, conversion rate/mode and completion handling; gain range is selected.
- Q10/Q21: calibrate sensor response and validate the complete measurement chain.

Electrical limits and gain scaling come from the [preserved TI ADS1115 datasheet](../../datasheets/ads1115-datasheet.pdf), especially recommended operating conditions and programmable gain. These are design calculations and requirements; no physical test results are recorded here.
