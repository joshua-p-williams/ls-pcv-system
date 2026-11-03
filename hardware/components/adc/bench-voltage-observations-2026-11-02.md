# ADC Bench Voltage Observations

These observations record a voltage-path check using battery cells as signal sources. They check whether the ADS1115 reading tracks voltage measured at its A0 input; they are not sensor calibration results and do not validate pressure conversion.

## Setup

- The N16R8 development board was powered from USB with its documented IN-OUT bridge closed and USB-OTG open.
- The ADS1115 was powered from the board's nominal 5 V rail and communicated over I²C.
- A battery positive lead was applied to the red signal lead / filter input and battery negative to the white common-ground lead. The FTP sensor was not connected; its yellow VCC lead remained disconnected.
- Voltage was read between ADS1115 G and A0 with a multimeter and compared with the firmware's `a0_uV` report.
- The diagnostic configuration was A0-to-GND, address `0x48`, PGA `+/-6.144 V` (187.5 µV/count), single-shot at 128 SPS, over 100 kHz I²C. These are firmware settings, not independently measured module properties.

## Results reported on 2026-11-02

| Signal source | Multimeter at ADS G–A0 | ADS raw code observed | ADS1115 reported voltage | Difference from representative reading |
|---|---:|---:|---:|---:|
| AAA cell | 1.603 V | 8541–8542 | 1.601438–1.601625 V | 1.375–1.562 mV |
| CR123A cell | 3.321 V | 17696–17697 | 3.318000–3.318188 V | 2.812–3.000 mV |

For the CR123A check, repeated firmware samples reported raw codes of 17696–17697 and `a0_uV` of 3,318,000–3,318,188 µV. The multimeter read 3.321 V across ADS G and A0. The difference between the representative readings is about 0.085% of the multimeter value.

An earlier unloaded battery reading for the CR123A was 3.322 V; the 3.321 V measurement above was taken at the ADC input during the connected test and is the relevant comparison.

## Interpretation and limits

The multimeter make/model, display resolution and accuracy specification were not recorded. The two points show that, in this bench setup, the ADC voltage report tracked the multimeter at A0 across the tested voltages; each steady ADC reading varied by one count or less in the supplied samples. This supports basic operation of the voltage acquisition path, including the ADC and signal connection used for this test. It does not establish the multimeter's accuracy, ADS1115 absolute accuracy, RC-filter frequency response, sensor output calibration, pressure conversion, or performance under vehicle electrical conditions. The reported voltage is rounded to the ADC driver's resolution and should not be interpreted as millivolt-level calibration.
