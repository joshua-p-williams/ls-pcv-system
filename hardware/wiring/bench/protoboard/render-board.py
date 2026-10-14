"""Render coordinate-accurate L04 SVGs from the validated solder-plan data."""
from pathlib import Path
import runpy
import html
import sys
CHECK_ONLY = globals().get("CHECK_ONLY", "--check" in sys.argv)
import xml.etree.ElementTree as ET

HERE = Path(__file__).resolve().parent
d = runpy.run_path(str(HERE / 'generate-plan.py'), init_globals={'CHECK_ONLY': CHECK_ONLY})
pins, groups, buses = d['pins'], d['groups'], d['buses']
cols = list('ABCDEFGHIJKLMNOPQRSTUVWXYZ') + ['A2']
colors = {'Black':'#263445','Red':'#d4353e','Orange':'#e98117','Green':'#12864d','Yellow':'#d6a600','Blue':'#287bd1','Violet':'#8643b9','White':'#ffffff','Brown':'#97532e','Pink':'#e95192','Gray':'#68788d'}
def xy(row,col):
    idx = cols.index(col) if isinstance(col,str) else col
    return (1160-40*idx,190+40*row)
external = {'FTP.A':(860,1190),'FTP.B':(960,1190),'FTP.C':(1060,1190)}
def point(key):
    if key in external:return external[key]
    row,col=pins[key][1].split(' / ')
    return xy(int(row),col)

# Intermediate route points are row/column-index positions; half steps route between pads.
# Only first/last points are solder joints. Route bends do not create new electrical nodes.
routes = {
 'G01':[(22,15.65),(4.45,15.65),(4.45,7)],
 'G02':[(4.4,12),(4.4,13.35),(1.6,13.35),(1.6,13)],
 'G03':[(5.2,9),(5.2,8.5),(9,8.5)],
 'G04':[(5.55,10),(5.55,12.45),(9,12.45)],
 'G05':[(4.6,11),(4.6,13.45),(16.5,13.45),(16.5,9)],
 'G06':[(17,6.5),(8,6.5)],
 'G07':[(8,4.5),(11,4.5)],
 'G08':[(17.5,7),(17.5,5)],
 'G09':[(15.5,8),(15.5,9)],
 'G10':[(22.35,16),(22.35,13.3),(20,13.3)],
 'G11':[(23.65,5),(23.65,7.5)],
 'P01':[(1,15.2),(2.4,15.2),(2.4,12)],
 'P02':[(2.05,13),(2.05,14)],
 'P03':[(3.55,14),(6,14),(6,11.5),(10,11.5)],
 'P04':[(21,15.05),(3.65,15.05),(3.65,6.5),(3,6.5)],
 'P05':[(2.55,8),(2.55,5),(7,5)],
 'P06':[(3.45,9),(3.45,8.65),(5.8,8.65),(5.8,7.5),(10,7.5)],
 'P07':[(3,3.5),(18,3.5)],
 'P08':[(18,1.5),(24.35,1.5),(24.35,2.5)],
 'I01':[(12,14.1),(12.45,14.1),(12.45,12)],
 'I02':[(12,6.25),(10,6.25)],
 'I03':[(15,13.8),(11.45,13.8),(11.45,12)],
 'I04':[(11,6.75),(9,6.75)],
 'D01':[(5,14.85),(1.35,14.85),(1.35,8)],
 'D02':[(16,14.5),(1.8,14.5),(1.8,9)],
 'D03':[(4,13.5),(2.6,13.5),(2.6,10)],
 'D04':[(17,13.95),(6.3,13.95),(6.3,10.6),(1,10.6)],
 'D05':[(18,13.6),(6.65,13.6),(6.65,11.7),(1,11.7)],
 'A01':[(23.2,5),(23.2,6.8),(18.5,6.8),(18.5,7)],
 'A02':[(18,6.1),(16,6.1)], 'A03':[], 'A04':[],
 'B01':[(6,14.95),(17.6,14.95),(17.6,7.7),(18,7.7)],
}
allrows = [row for _,_,rows in groups for row in rows]
assert set(routes)=={r[0] for r in allrows}
views = {
 'board-overview':('Complete wiring', {r[0] for r in allrows}, {'G','P5','P3','AG'}),
 'board-ground-power':('Ground and power', {r[0] for r in allrows if r[0][0] in 'GP'}, {'G','P5','P3','AG'}),
 'board-digital':('I2C and display signals', {r[0] for r in allrows if r[0][0] in 'ID'}, set()),
 'board-sensor-button':('Sensor, RC filter and button', {'G05','G06','G08','G09','G10','G11','P07','P08','A01','A02','A03','A04','B01'}, {'G','P5','AG'}),
}

def esc(s):return html.escape(str(s),quote=True)
def render(name,title,selected,active_buses):
    s=[]
    def add(v):s.append(v)
    def text(x,y,value,size=16,fill='#25364a',anchor='start',weight=400,extra=''):
        add(f'<text x="{x}" y="{y}" font-size="{size}" fill="{fill}" text-anchor="{anchor}" font-weight="{weight}" {extra}>{esc(value)}</text>')
    def rect(x,y,w,h,fill,stroke='none',r=8,extra=''):
        add(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{r}" fill="{fill}" stroke="{stroke}" {extra}/>')
    def circle(x,y,r,fill,stroke='none',width=1):add(f'<circle cx="{x}" cy="{y}" r="{r}" fill="{fill}" stroke="{stroke}" stroke-width="{width}"/>')
    def line(points,color,width=2,extra=''):
        p=' '.join(f'{x},{y}' for x,y in points)
        add(f'<polyline points="{p}" fill="none" stroke="{color}" stroke-width="{width}" stroke-linecap="round" stroke-linejoin="round" {extra}/>')
    add(f'<svg xmlns="http://www.w3.org/2000/svg" width="1900" height="1410" viewBox="0 0 1900 1410" role="img" aria-labelledby="title desc">')
    add(f'<title id="title">Solder-side bench board: {esc(title)}</title><desc id="desc">Proposed wire routing on a 27-column by 22-row isolated-pad carrier. Coordinates follow supplied solder-side labels. Physical footprints are schematic projections. Contact check required for button.</desc>')
    add('<style>text{font-family:Segoe UI,Arial,sans-serif}.wire:hover{filter:drop-shadow(0 0 3px #233)}.wire{cursor:help}</style>')
    rect(0,0,1900,1410,'#f4f6f9',r=0)
    text(55,55,'PCV / BENCH BUILD',18,'#477186',weight=700)
    text(55,101,title,38,weight=700)
    text(55,136,'SOLDER SIDE  •  Rows 01–22  •  Columns A → Z → A2 run RIGHT TO LEFT',18)
    text(55,165,'Proposed routing • exact solder-pad coordinates • component bodies shown only as location guides',16,'#5e6e7c')
    rect(74,192,1130,916,'#e9f2ed','#9bad9f',18)
    # Module projections, beneath grid and traces.
    rect(100,207,440,886,'#dce9f4','#7996b4',10,extra='stroke-dasharray="7 5"')
    text(303,520,'J1',40,'#6e8ba7','middle',700)
    text(303,555,'ESP32-S3 N16R8',23,'#6e8ba7','middle',700)
    text(303,583,'header projection',16,'#6e8ba7','middle')
    text(303,613,'component is on the far side',15,'#6e8ba7','middle')
    rect(945,450,106,399,'#eee1f0','#a785b2',8,extra='stroke-dasharray="5 4"')
    text(997,442,'J3 / ADC',17,'#745b80','middle',700)
    rect(662,450,196,240,'#dae9f1','#81a4ba',8,extra='stroke-dasharray="5 4"')
    text(760,442,'LS1',17,'#486f86','middle',700)
    rect(582,211,276,36,'#e3e6f7','#939aba',5)
    text(720,201,'J2 / display header',15,'#565d89','middle',700)
    rect(622,889,236,123,'#eee6cf','#a49364',10,extra='stroke-dasharray="5 4"')
    circle(740,950,29,'#e2d4ac','#a49364',2)
    text(740,944,'S1',19,'#776539','middle',700)
    text(740,966,'BUTTON',11,'#776539','middle',700)
    # All pads; visible grid does not imply connected strips.
    for row in range(1,23):
        for col in cols:
            x,y=xy(row,col);circle(x,y,5.5,'#f9fbfa','#b5c6bb',1)
        text(58,xy(row,'A')[1]+6,f'{row:02}',18,'#526c5f','end',600)
        text(1220,xy(row,'A')[1]+6,f'{row:02}',18,'#526c5f',weight=600)
    for col in cols:
        x,_=xy(1,col);text(x,184,col,16,'#526c5f','middle',700);text(x,1137,col,16,'#526c5f','middle',700)
    # Faint occupied rings remain visible in sectional drawings.
    for key,(label,coord) in pins.items():
        if coord=='Off-board':continue
        x,y=point(key);circle(x,y,8,'#f6f8fa','#647384',1.5)
    for i,label in enumerate(d['left'],1):
        x,y=point(f'J1.L{i}');text(x-13,y+5,label,13,'#536c85','end')
    for i,label in enumerate(d['right'],1):
        x,y=point(f'J1.R{i}');text(x+13,y+5,label,13,'#536c85')
    for label in ['V','G','SCL','SDA','ADDR','ALERT','A0','A1','A2','A3']:
        x,y=point('J3.'+label);text(x+13,y+5,label,13,'#795786',weight=600)
    for side in ['HV','LV']:
        for label in [side+'4',side+'3','GND_'+side,side,side+'2',side+'1']:
            x,y=point('LS1.'+label);text(x+13 if side=='LV' else x-13,y+5,'GND' if label.startswith('GND') else label,12,'#416a80','start' if side=='LV' else 'end',600)
    for label in ['RST','CS','DC','SDA','SCL','GND','VCC']:
        x,y=point('J2.'+label);text(x,y+26,label,10,'#525c84','middle',700)
    # External sensor remains outside carrier, not a fictitious board footprint.
    rect(800,1190,320,108,'#ffffff','#9aacbd',10)
    text(960,1231,'FTP sensor / off-board',21,anchor='middle',weight=700)
    text(960,1260,'Pigtail cavity IDs; not wire colors',14,'#68788d','middle')
    for key in external:
        x,y=point(key);circle(x,y,7,'#fff','#6b7d8f',2);text(x,1211,key[-1],13,anchor='middle',weight=700)
    # Bus copper: only these strips are intentionally bridged pad-to-pad.
    for bus,row,cs,label in buses:
        active=bus in active_buses
        color=colors['Red' if bus=='P5' else 'Orange' if bus=='P3' else 'Black'] if active else '#c6cdd1'
        pts=[xy(row,c) for c in cs]
        line(pts,'#fff',11);line(pts,color,7)
        for x,y in pts:circle(x,y,4,color)
    # Draw routed wires with halos: crossing traces have no solder connection.
    for ident,a,b,color,why in allrows:
        if ident not in selected:continue
        pts=[point(a)]+[xy(*p) for p in routes[ident]]+[point(b)]
        assert pts[0]==point(a) and pts[-1]==point(b)
        add(f'<g class="wire" id="{ident}" data-start="{esc(a)}" data-end="{esc(b)}"><title>{ident}: {esc(pins[a][0])} ({pins[a][1]}) → {esc(pins[b][0])} ({pins[b][1]}); {color}. {esc(why)}</title>')
        line(pts,'#f8faf9',9);line(pts,'#607080' if color=='White' else colors[color],6);line(pts,colors[color],3.8)
        add('</g>')
    # R1 and C1 are symbols between actual lead coordinates, not copper shorts.
    x,y1=point('R1.OUT');_,y2=point('R1.IN')
    line([(x,y1),(x,y1+27)],'#695035',3);line([(x,y2-27),(x,y2)],'#695035',3)
    rect(x-9,y1+27,18,y2-y1-54,'#dfc797','#806332',3)
    for yy,cc in [(y1+38,'#e5bf24'),(y1+47,'#8748a7'),(y1+56,'#875528'),(y2-36,'#aa842a')]:line([(x-8,yy),(x+8,yy)],cc,4)
    text(x+31,y1+66,'R1',14,weight=700);text(x+31,y1+84,'470 Ω',13)
    x,ya=point('C1.OUT');_,yb=point('C1.GND');mid=(ya+yb)/2
    line([(x,ya),(x,mid-5)],'#516174',3);line([(x,mid+5),(x,yb)],'#516174',3)
    line([(x-13,mid-5),(x+13,mid-5)],'#516174',4);line([(x-13,mid+5),(x+13,mid+5)],'#516174',4)
    text(x-22,mid,'C1',14,anchor='end',weight=700);text(x-22,mid+18,'1 µF',13,anchor='end')
    # Endpoint dots are drawn last. Only these points and explicit bus taps are joints.
    used=set()
    for ident,a,b,color,why in allrows:
        if ident in selected: used.update([a,b])
    for key in sorted(used):
        x,y=point(key);circle(x,y,6,'#fff','#263445',2);circle(x,y,2.5,'#263445')
    for key in ['S1.LEG_18N','S1.LEG_20I']:
        x,y=point(key);circle(x,y,7,'#eee6cf','#8e7c50',2)
    # Sidebar: compact exact schedule, color swatches and build constraints.
    rect(1270,192,580,1112,'#fff','#d6dfe8',16)
    text(1300,229,'WIRE INDEX',18,weight=700)
    text(1300,255,'IDs match solder-plan.md • hover SVG wires for details',14,'#66768b')
    text(1350,290,'FROM  →  TO',13,'#65758a',weight=700)
    yy=318
    for ident,a,b,color,why in allrows:
        if ident not in selected:continue
        line([(1300,yy-5),(1326,yy-5)],'#607080',6);line([(1300,yy-5),(1326,yy-5)],colors[color],4)
        text(1340,yy,ident,14,weight=700)
        def short(key):
            coord=pins[key][1].replace(' / ','/')
            return key if coord=='Off-board' else coord
        text(1387,yy,f'{short(a)} → {short(b)}',15)
        text(1600,yy,color,13,'#66768b')
        yy+=20
    yy=max(yy+28,665)
    text(1300,yy,'READ THIS VIEW',16,weight=700);yy+=28
    for note in ['Black-centered dots = wire endpoints / solder joints.',
                 'Crossings and route bends are NOT connections.',
                 'Bus strips are explicit links; other pads are isolated.',
                 'R1/C1 symbols connect leads through components.',
                 'Button: verify 18/I–20/N opens/releases, closes/presses.',
                 'Extra switch legs: support solder only; no added wire.',
                 'Bodies are location guides, not measured outlines.',
                 'Wire routes are suggestions, not cut-length templates.']:
        text(1300,yy,note,14,'#516176');yy+=24
    text(1300,yy+15,f'{len(selected)} wires shown / {len(allrows)} total',15,weight=700)
    text(105,1190,'BUS LOCATIONS',17,weight=700)
    for yy,label in [(1220,'GND: 04/H–N     •     AG: 17/H–J'),(1247,'5 V: 03/H–K     •     3.3 V: 03/M–O'),(1274,'Keep 03/L isolated. FTP 5 V uses 18/D, not 18/J.')]:text(105,yy,label,16)
    line([(55,1332),(1845,1332)],'#cbd6e0',1)
    text(55,1365,'USB only • IN-OUT closed / USB-OTG open • GPIO19/20 reserved • no firmware or physical tests implied',16,'#50647a')
    text(55,1390,'L04 build aid derived from W01 and the supplied solder-side map. Check the accompanying solder schedule before applying power.',14,'#66768b')
    add('</svg>')
    dest=HERE/'generated'/(name+'.svg')
    content='\n'.join(s)
    if CHECK_ONLY:
        if not dest.exists() or dest.read_text(encoding='utf-8') != content:
            raise SystemExit(f"Stale L04 SVG: {dest.name}; run render-board.py and export-png.mjs")
    else:
        dest.parent.mkdir(parents=True,exist_ok=True)
        dest.write_text(content,encoding='utf-8',newline='\n')
    root=ET.parse(dest).getroot()
    rendered={g.attrib['id'] for g in root.findall('{http://www.w3.org/2000/svg}g') if g.attrib.get('class')=='wire'}
    assert rendered==selected
    print(f'{dest.name}: {len(rendered)} exact-endpoint wires; SVG XML valid')

for name,(title,selected,active) in views.items():render(name,title,selected,active)
