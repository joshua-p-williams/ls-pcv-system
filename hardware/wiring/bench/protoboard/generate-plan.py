"""Generate the L04 solder schedule and compare its copper nets with W01."""
from pathlib import Path
import sys
CHECK_ONLY = globals().get("CHECK_ONLY", "--check" in sys.argv)
import yaml

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]
pins = {}
def pin(key, label, row=None, col=None):
    pins[key] = (label, f"{row:02d} / {col}" if row is not None else "Off-board")
left = ['3V3','3V3','RST','GPIO4','GPIO5','GPIO6','GPIO7','GPIO15','GPIO16','GPIO17','GPIO18','GPIO8','GPIO3','GPIO46','GPIO9','GPIO10','GPIO11','GPIO12','GPIO13','GPIO14','5Vin','GND']
right = ['GND','TX','RX','GPIO1','GPIO2','GPIO42','GPIO41','GPIO40','GPIO39','GPIO38','GPIO37','GPIO36','GPIO35','GPIO0','GPIO45','GPIO48','GPIO47','GPIO21','GPIO20','GPIO19','GND','GND']
for i, label in enumerate(left, 1): pin(f'J1.L{i}', f'ESP32 {label} (J1.L{i})', i, 'Q')
for i, label in enumerate(right, 1): pin(f'J1.R{i}', f'ESP32 {label} (J1.R{i})', i, 'A2')
for i, label in enumerate(['V','G','SCL','SDA','ADDR','ALERT','A0','A1','A2','A3'], 7): pin('J3.'+label, 'ADC '+label, i, 'F')
for col, label in zip('IJKLMNO', ['RST','CS','DC','SDA','SCL','GND','VCC']): pin('J2.'+label,'Display '+label,1,col)
for side, col in [('HV','I'),('LV','M')]:
    for row, label in enumerate([side+'4',side+'3','GND_'+side,side,side+'2',side+'1'],7): pin('LS1.'+label,'Translator '+label,row,col)
for key,label,row,col in [
    ('R1.IN','R1 sensor-side lead',16,'H'),('R1.OUT','R1 ADC-side lead',13,'H'),
    ('C1.OUT','C1 signal-side lead',13,'J'),('C1.GND','C1 ground-side lead',15,'J'),
    ('T.GND','FTP-GND landing pad',18,'F'),('T.SIG','FTP-SIG landing pad',18,'H'),('T.5V','FTP-5V landing pad',18,'D'),
    ('S1.CONTACT_A','Button signal leg (check switched pair)',18,'I'),
    ('S1.CONTACT_B','Button ground leg (check switched pair)',20,'N'),
    ('S1.LEG_18N','Button extra leg; no added wire',18,'N'),
    ('S1.LEG_20I','Button extra leg; no added wire',20,'I')]: pin(key,label,row,col)
for key,label in [('FTP.A','FTP pigtail A / ground'),('FTP.B','FTP pigtail B / signal'),('FTP.C','FTP pigtail C / 5 V')]: pin(key,label)
buses = [('G',4,'HIJKLMN','Ground'),('P5',3,'HIJK','Nominal 5 V'),('P3',3,'MNO','3.3 V'),('AG',17,'HIJ','Analog ground')]
for name,row,cols,label in buses:
    for col in cols: pin(name+col,label+' bus tap',row,col)
groups = []
def group(title, explanation, rows): groups.append((title,explanation,rows))
group('1. Common ground and address — black',
'''Ground gives all modules the same zero-volt reference and a return path. The separate AG bus joins sensor, ADC and capacitor returns locally; it is wired to the main ground bus, not isolated from it. The display has its own return to the main bus. Build the ground and AG bus links listed above first.''',[
('G01','J1.L22','GH','Black','Main carrier return to controller/USB ground'),
('G02','GM','J2.GND','Black','Display power return from the nearest available main-ground tap'),
('G03','GJ','LS1.GND_HV','Black','Translator high-side ground pad'),
('G04','GK','LS1.GND_LV','Black','Translator low-side ground pad; same ground net'),
('G05','GL','AGJ','Black','Connect local analog return to main ground'),
('G06','AGH','J3.G','Black','ADC supply return and analog reference'),
('G07','J3.G','J3.ADDR','Black','Select candidate I2C address 0x48; check for contradictory existing hard strap'),
('G08','AGH','T.GND','Black','Sensor ground landing pad'),
('G09','AGI','C1.GND','Black','Short local filter-capacitor return'),
('G10','J1.L22','S1.CONTACT_B','Black','Short button return directly to controller ground; same common-ground net'),
('G11','T.GND','FTP.A','Black','Off-board sensor low-reference lead'),
])
group('2. Supplies — orange 3.3 V, red nominal 5 V',
'''The controller's regulator supplies 3.3 V to the display and translator LV reference. Its USB-fed 5Vin pin supplies the ADC, sensor and translator HV reference. The translator does not generate either voltage. Build the two separate supply buses first; row 03/L stays isolated between them.''',[
('P01','J1.L1','P3M','Orange','Controller-regulated 3.3 V distribution'),
('P02','P3N','J2.VCC','Orange','Display supply at selected 3.3 V'),
('P03','P3O','LS1.LV','Orange','Translator low-side voltage reference'),
('P04','J1.L21','P5H','Red','USB-derived nominal 5 V distribution; requires IN-OUT closed'),
('P05','P5I','J3.V','Red','ADC supply for full sensor voltage range'),
('P06','P5J','LS1.HV','Red','Translator high-side voltage reference'),
('P07','P5H','T.5V','Red','Sensor supply landing pad from the nearer end of the shared 5 V bus'),
('P08','T.5V','FTP.C','Red','Off-board sensor supply'),
])
group('3. ADC I2C — green data, yellow clock',
'''I2C carries configuration commands and ADC readings. Channel 1 translates SDA data; channel 2 translates SCL clock. Devices pull the bus low or release it, while pull-ups establish different high voltages on the two sides. Do not directly bridge LV1-HV1 or LV2-HV2; the module performs that translation. No additional pull-ups are specified beyond fitted module resistors.''',[
('I01','J1.L12','LS1.LV1','Green','3.3 V controller-side I2C data / GPIO8'),
('I02','LS1.HV1','J3.SDA','Green','5 V ADC-side I2C data'),
('I03','J1.L15','LS1.LV2','Yellow','3.3 V controller-side I2C clock / GPIO9'),
('I04','LS1.HV2','J3.SCL','Yellow','5 V ADC-side I2C clock'),
])
group('4. Display SPI and controls',
'''The display's SDA/SCL labels mean SPI data and clock here, not I2C. These five signals connect directly to ESP32 3.3 V GPIOs, without the translator. Label the green/yellow pair DISPLAY to distinguish it from ADC I2C. There is no additional MISO or backlight wire.''',[
('D01','J1.L5','J2.RST','White','GPIO5 resets the display; not the ESP32 RST pin'),
('D02','J1.L16','J2.CS','Blue','GPIO10 selects the display during transfer'),
('D03','J1.L4','J2.DC','Violet','GPIO4 distinguishes command bytes from display data'),
('D04','J1.L17','J2.SDA','Green','GPIO11 sends SPI commands/pixels (MOSI)'),
('D05','J1.L18','J2.SCL','Yellow','GPIO12 clocks SPI transfers'),
])
group('5. Pressure signal and RC filter',
'''The 470 ohm resistor is in series with the sensor output. The 1 uF capacitor runs from the resistor's ADC end to ground. This low-pass filter reduces rapid voltage noise; it does not divide the signal voltage. Its nominal ideal time constant is 0.47 ms and cutoff about 339 Hz, not a measured system response. Keep these pink wires short and away from the display clock group where practical.''',[
('A01','FTP.B','T.SIG','Brown','Off-board raw sensor signal; identify factory lead by cavity'),
('A02','T.SIG','R1.IN','Brown','Connect raw signal to input end of resistor'),
('A03','R1.OUT','J3.A0','Pink','Short filtered signal connection to ADC A0'),
('A04','R1.OUT','C1.OUT','Pink','Join capacitor signal lead to the same filtered node'),
])
group('6. Multifunction button',
'''The on-board normally-open button connects GPIO6 to ground when pressed. Firmware will enable its internal pull-up and interpret gestures; neither is implemented yet. The selected diagonal legs 18/I and 20/N must be open when released and connected when pressed: check the loose switch before wiring. If they are already connected, do not fit this pair's wires; identify the actual switched pair and update its coordinates first. The other two legs receive solder for support but no added wire; they may still be internally connected to the used contacts. G10 supplies the ground wire.''',[
('B01','J1.L6','S1.CONTACT_A','Gray','GPIO6 directly to on-board button signal contact'),
])

class Nets:
    def __init__(self): self.parent={}
    def find(self,x):
        self.parent.setdefault(x,x)
        if self.parent[x]!=x:self.parent[x]=self.find(self.parent[x])
        return self.parent[x]
    def join(self,a,b):self.parent[self.find(a)]=self.find(b)
actual=Nets()
for name,row,cols,label in buses:
    for a,b in zip(cols,cols[1:]):actual.join(name+a,name+b)
for _,_,rows in groups:
    for _,a,b,_,_ in rows: actual.join(a,b)
aliases={'RAILS.GND':'GH','RAILS.3V3':'P3M','RAILS.5V':'P5H','COND.IN':'R1.IN','COND.OUT':'R1.OUT','COND.GND':'C1.GND'}
expected=Nets()
source=yaml.safe_load((ROOT/'hardware/wiring/bench/bench-harness.yml').read_text(encoding='utf-8'))
color_names={k:v['name'] for k,v in source['x-w01-color-convention'].items()}
for connection in source['connections']:
    (a,aps), (bundle,nums), (b,bps)=[next(iter(x.items())) for x in connection]
    if bundle=='W_USB':continue
    for ap,bp,num in zip(aps,bps,nums):
        x,y=f'{a}.{ap}',f'{b}.{bp}'
        expected.join(x,y)
        # Each added wire on this logical net must preserve its W01 color.
        color=color_names[source['cables'][bundle]['colors'][num-1]]
        for _,_,rows in groups:
            for _,s,t,c,_ in rows:
                if actual.find(aliases.get(x,x)) == actual.find(s): assert c==color,(bundle,c,color)
terminals=list(expected.parent)
for a in terminals:
    for b in terminals:
        assert (expected.find(a)==expected.find(b)) == (actual.find(aliases.get(a,a))==actual.find(aliases.get(b,b))),(a,b)
assert actual.find('C1.OUT')==actual.find('J3.A0')
assert actual.find('C1.GND')==actual.find('J3.G')
assert actual.find('R1.IN')!=actual.find('R1.OUT')
assert actual.find('C1.OUT')!=actual.find('C1.GND')
occupied={}
for key,(label,coord) in pins.items():
    if coord=='Off-board':continue
    assert coord not in occupied,(key,occupied.get(coord),coord)
    occupied[coord]=key
    row,col=coord.split(' / ')
    assert 1<=int(row)<=22 and col in list('ABCDEFGHIJKLMNOPQRSTUVWXYZ')+['A2']

def write(name,text):
    path=HERE/name
    if CHECK_ONLY:
        if not path.exists() or path.read_text(encoding='utf-8') != text:
            raise SystemExit(f"Stale L04 document: {name}; run render-board.py and export-png.mjs")
    else:
        path.write_text(text,encoding='utf-8',newline='\n')
def table(rows):return '\n'.join('| '+' | '.join(row)+' |' for row in rows)+'\n'
intro='''# Solder-side bench wiring plan

**Use the printed solder-side coordinates for every hole in this document.** The corrected map supplied on 2026-10-13 supersedes the earlier mixed-face coordinates. Rows run 01-22 from the display/antenna end toward the USB end. Column **A2** means the repeated A after Z; it is the ESP32's other header, not the first A. Do not translate these letters using the component-side printing.

J1.L names retain the project’s component-front-view header naming even while soldering from the back. Thus J1.L1 is the 3V3 pin at **01/Q**, J1.L21 is 5Vin at **21/Q**, and J1.L22 is GND at **22/Q**. The other ESP32 header is column A2. All selected gauge wires use column Q.

Existing module coordinates are supplied physical locations. New bus, RC and cable-landing locations below are **proposed placements**, not claims that they are already fitted. Pads are individually isolated. The schedule preserves W01 electrical nets, GPIOs and colors. It is a bench construction plan; physical operation and firmware remain untested.

## Complete wiring overview

![Complete solder-side grid and colored wiring](generated/board-overview.png)

[Open zoomable SVG](generated/board-overview.svg). The sectional images below show the same routing with other circuits omitted. Crossings are insulated; only designated endpoints and bus strips are joined. Physical bend locations are suggestions, not a cut-length template.

### Routing audit changes

Three wire starts were optimized without changing electrical nets: **G02 now starts at ground tap 04/M (was 04/I)**; **G10 now starts at ESP32 GND 22/Q (was ground tap 04/N)**; **P07 now starts at 5 V tap 03/H (was 03/K)**. All destination pins, GPIOs, component placements and bus strips remain as listed. If earlier wiring is already installed, those former same-net starts remain electrically valid; rerouting them is optional.

G01 and G10 share the controller's 22/Q ground solder node. P04 and P07 share the 03/H 5 V bus tap. Make a supported solder junction at each, without forcing multiple wires through a single hole. Shorter suggested bends also simplify the I2C and button signal routes.

## Parts and placement

| Part | Value / options | First lead, solder-side row / column | Second lead, solder-side row / column |
| --- | --- | --- | --- |
| R1 | 470 ohm, 1/4 W axial; 1% metal-film preferred, 5% carbon-film acceptable; 1/8 W also sufficient for normal signal use | IN: **16/H** | OUT: **13/H** |
| C1 | 1 uF nonpolar film **or X7R ceramic**, rated at least 10 V; 16/25/50 V also fine | Signal: **13/J** | Ground: **15/J** |

Fit the bodies on the component side and solder their leads at those solder-side hole labels. R1 spans three 2.54 mm intervals (7.62 mm); C1 spans two (5.08 mm). Bend leads gently to suit, leaving clearance at the body. Neither selected part is polarized. A bulky film capacitor can stand above the board on supported/insulated leads; do not force a package into this footprint or let its leads touch adjacent pads. If a particular stock part cannot fit, relocate its holes in this plan before soldering; its value and circuit need not change.

470 ohm examples: four bands yellow-violet-brown-gold (5%), or five bands yellow-violet-black-black-brown (1%). Verify with the meter before fitting. A capacitor marked **105** is nominally 1 uF, but that code does not establish its voltage rating or dielectric. Use packaging/part information for those. Do not substitute a polarized electrolytic/tantalum for the selected nonpolar C1.

R1 is the only connection between IN and OUT. C1 is the only component between its signal lead and ground. **Do not install a jumper across either component.** C1 is a signal filter, not the ADC's supply bypass capacitor. The ADC's +/-6.144 V gain setting does not permit an A0 voltage outside its ground-to-supply range.

### On-board button placement

The reported footprint has four empty holes between legs in one direction and one empty hole in the other: five and two grid intervals, nominally **12.7 mm x 5.08 mm** on this carrier. Mount the button on the component side with its leads through solder-side holes **18/I, 18/N, 20/I and 20/N**. Dry-fit the body before soldering; the footprint does not establish the casing dimensions. Keep its casing clear of R1/C1 and insulate any nearby exposed metal. Bus spines and sensor wire joints in this area belong on the solder side, not under the button body on top.

Use **18/I for signal** and **20/N for ground**, conditional on the unpowered contact check in section 6. Solder the other legs into their isolated pads for support with no added wires. No external button cable or separate button landing pads are needed; former 20/M and 20/O landing pads are released. G12 and B02 from the earlier off-board schedule are retired; the other circuit IDs stay stable.

**Placement revision:** move the proposed FTP-5V landing pad from **18/J to 18/D** to keep the supply termination outside the button footprint. Both red wires P07/P08 now use 18/D. The old 18/J pad stays unconnected. If wires were already fitted there, disconnect power and relocate both before fitting the button. FTP-GND remains 18/F and FTP-SIG remains 18/H; route and support these leads on the solder side with clearance from switch legs.

## Build these short distribution buses first

These are new copper links you add, not existing board traces. Join every adjacent pair specified below. Prefer a solid wire spine tacked to each pad, with stripped tap ends soldered to the spine; do not rely on a long unsupported solder blob. Insulate crossings and exposed wire outside the intended bus. More than one lead may join a node at the solder side without forcing several conductors through one hole.

| Bus | Explicit pad-to-pad links, all solder-side | Color | Purpose |
| --- | --- | --- | --- |
| G / main ground | 04/H–04/I; 04/I–04/J; 04/J–04/K; 04/K–04/L; 04/L–04/M; 04/M–04/N | Black | Main return distribution |
| P5 / nominal 5 V | 03/H–03/I; 03/I–03/J; 03/J–03/K | Red | Shared sensor/ADC/translator HV supply |
| P3 / 3.3 V | 03/M–03/N; 03/N–03/O | Orange | Display and translator LV supply |
| AG / analog ground | 17/H–17/I; 17/I–17/J | Black | Local ADC/filter/sensor returns |

**Leave 03/L unconnected** between the two supply buses. Do not join row 03 to row 04. AG is the same electrical ground as G, connected by G05 below. Ground taps 04/I and 04/N and the 5 V tap 03/K have no outgoing wires in the optimized schedule; retain their bus links as specified.

## Off-board connection landing pads

| Label | Solder-side row / column | Added lead color | External endpoint |
| --- | --- | --- | --- |
| FTP-GND | 18/F | Black | FTP pigtail A |
| FTP-SIG | 18/H | Brown | FTP pigtail B |
| FTP-5V | 18/D | Red | FTP pigtail C |

The **FTP sensor stays off-board**. These are separate landing pads, not an assumed standard connector pin order. Leave the intervening holes isolated. Support/strain-relieve the cable before its solder joints; use secure removable connections if convenient. The button is on-board at the four holes specified above.

At the FTP pigtail **mating face**, looking into its terminal openings with latch up: A is left/ground, B center/signal, C right/5 V, per the adopted family map. The wire-entry view reverses left/right. Trace cavities to the free leads with the disconnected pigtail and label them. Listed wire colors apply to added wiring, not factory pigtail insulation.

## Wire schedule

Every row is one wire. Bus-spine links are separately listed above. R1/C1 are installed parts, not omitted jumper wires. Use insulated 22 AWG as a practical starting choice for main feeds and display power, 24 AWG for short signal/module leads, subject to pad fit and actual load. Cut to fit with modest service slack. Record and label any color substitutions.

'''
out=intro
for title,explanation,rows in groups:
    out+=f'### {title}\n\n{explanation}\n\n'
    preview = {'1.':'board-ground-power','3.':'board-digital','5.':'board-sensor-button'}.get(title[:2])
    if preview:
        out+=f'![{title[3:]} — solder-side routing](generated/{preview}.png)\n\n[Open zoomable SVG](generated/{preview}.svg). '
        out+= {'1.':'This view covers ground and supplies in sections 1–2.', '3.':'This view covers I2C and display connections in sections 3–4.', '5.':'This view covers the sensor/filter and button in sections 5–6, with their related supply/return wires.'}[title[:2]]+'\n\n'
    out+=table([['Circuit','Start component pin / node','Start row / column','Wire color','End component pin / node','End row / column','Function'],['---']*7]+[[id,pins[a][0],pins[a][1],color,pins[b][0],pins[b][1],why] for id,a,b,color,why in rows])+'\n'
out+='''## Power jumper and checks

1. Solder and check with all power unplugged. Complete the mechanical wiring in the order above; no firmware is needed to build it. Removable modules can be taken out for socket soldering and bare-carrier short checks.
2. The selected ESP32 power setup requires **IN-OUT closed, USB-OTG open**. Close only the labeled IN-OUT pads if needed; do not bridge other jumpers. These are on the ESP32 module, not carrier holes. Use native USB (left port in the original component-front photo); its apparent side reverses when viewed from behind. COM remains disconnected. One computer or standalone USB source at a time; no simultaneous external 5 V injection.
3. Inspect continuity along every intended wire and bus; check adjacent pads for unintended bridges. Check no copper short between 3.3 V, nominal 5 V, ground, raw signal and filtered signal. R1 IN-to-OUT should be about 470 ohm with parallel circuitry isolated. C1 can produce a transient charging indication; it must not be bypassed by a sustained copper short. Meter readings through fitted modules need interpretation.
4. With peripherals disconnected/removed, power the controller and measure 3V3 and 5Vin against ground. The selected family circuit has a USB input diode; nominal 5 V is not necessarily 5.000 V. Disconnect power before installing modules. Then check actual rail voltage/load with peripherals, including sensor and ADC supply at their endpoints.
5. Before communication, verify the translator LV/HV supplies and respective idle bus levels. Start with the fitted pull-ups; communication at the proposed 100 kHz requires firmware. Keep sensor and ADC powered from the same branch; no independently powered sensor feeding an unpowered ADC is selected.
6. Leave ADC ALERT and A1/A2/A3 externally unconnected for this A0-only build. Leave translator channels 3/4 externally unconnected. Do not add gauge wiring to ESP32 GPIO19/20 (native USB) or GPIO35/36/37 (N16R8 memory use). The second 3V3 and other unused headers need no extra wires.
7. Display output, button gestures and ADC readings require firmware that has not yet been implemented. Continuity and voltage checks validate only those measured aspects, not calibration or complete operation.

Route insulated wires on the solder side without obstructing sockets or USB access. Use the open gap between modules for the translated I2C connections. Run the display group toward the top; keep the short pink filter wiring local. A crossing is not an electrical junction unless explicitly soldered. Preserve module removal and support the protruding display/controller mechanically.

## Evidence and maintenance

- [Full component coordinate map](coordinate-map.md): supplied solder-side positions plus proposed new parts/pads.
- [W01 source](../../../../hardware/wiring/bench/bench-harness.yml) and [connection schedule](../../../../hardware/wiring/bench/generated/connections.md): connectivity and color authority. The buses and landing pads subdivide its connections without changing their electrical nets.
- [Controller power plan](../../../../hardware/components/esp32-s3-dev-board/power-plan.md), [ADC pinout](../../../../hardware/pinouts/ads1115.md), [display pinout](../../../../hardware/pinouts/gc9a01.md), [translator](../../../../hardware/components/i2c-level-shifter/README.md), [FTP pinout](../../../../hardware/pinouts/ftp-sensor.md), [RC filter](../../../../hardware/components/rc-input-filter/README.md).
- Published references rechecked during preparation: [TI ADS1115](https://www.ti.com/lit/ds/symlink/ads1115.pdf), [board designer](https://github.com/vcc-gnd/YD-ESP32-S3), [Espressif GPIO restrictions](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/gpio.html), [Philips translation explanation](https://cdn-shop.adafruit.com/datasheets/an97055.pdf), [purchased display listing](https://www.amazon.com/dp/B0DYP4J9XP), and [purchased translator listing](https://www.amazon.com/dp/B0DSZBC8K6). Exact purchased carrier circuitry remains the documented family assumption.

The generator `generate-plan.py` produces both Markdown files and checks occupied-pad uniqueness, board bounds, wire colors and electrical-net equivalence against all non-USB W01 connections. R1/C1 topology is checked separately; they are not treated as copper shorts. Run `render-board.py` and `export-png.mjs` after changing this plan; see [reproduction instructions](README.md#reproduce-and-check). These are document checks, not physical fit or electrical tests. Reported module coordinates and proposed placements have separate evidence states; see [provenance](README.md#import-provenance).
'''
write('solder-plan.md',out)
record='''# Solder-side coordinate record

The 2026-10-13 corrected solder-side map supersedes all earlier mixed-face coordinates. Use printed solder-side labels only. A2 is the repeated A after Z. Pads are individually isolated.

ESP32 left header (project J1.L, named from component front) is column Q, rows 01-22. Its other header is column A2. ADC, display and translator entries follow the supplied full maps. R1/C1, button, bus taps and sensor-landing pads are proposed new placements; check stock-part body fit before soldering. Button contact assignments require the released/pressed continuity check, not an assumed internal pinout. FTP-5V has moved from 18/J to 18/D to clear the button. Former button landing pads 20/M and 20/O are released. Off-board sensor terminals intentionally have no carrier coordinate.

'''
record+=table([['ID','Component terminal / node','Solder-side row / column'],['---']*3]+[[k,*v] for k,v in pins.items()])
record+='\nSee [solder plan](solder-plan.md) for every wire, bus link, part value and circuit explanation.\n'
write('coordinate-map.md',record)
print(f'PASS: {len(occupied)} unique occupied/proposed pads; {sum(len(r) for _,_,r in groups)} wires; all non-USB W01 nets and wire colors preserved; RC copper topology checked.')
