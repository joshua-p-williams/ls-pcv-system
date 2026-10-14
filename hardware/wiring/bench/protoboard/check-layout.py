"""Check rendered paths for accidental passage over unrelated occupied pads."""
from pathlib import Path
import runpy
import math
import re
import json
import hashlib
HERE=Path(__file__).resolve().parent
r=runpy.run_path(str(HERE/'render-board.py'), init_globals={'CHECK_ONLY': True})
def distance(p,a,b):
    dx,dy=b[0]-a[0],b[1]-a[1]
    n=dx*dx+dy*dy
    t=max(0,min(1,((p[0]-a[0])*dx+(p[1]-a[1])*dy)/n)) if n else 0
    return math.hypot(p[0]-a[0]-t*dx,p[1]-a[1]-t*dy)
problems=[]
for ident,a,b,color,why in r['allrows']:
    points=[r['point'](a)]+[r['xy'](*p) for p in r['routes'][ident]]+[r['point'](b)]
    for key,(_,coord) in r['pins'].items():
        if key in [a,b] or coord=='Off-board':continue
        p=r['point'](key)
        closest=min(distance(p,u,v) for u,v in zip(points,points[1:]))
        if closest<9:
            problems.append(f'{ident} passes within {closest:.1f}px of unrelated endpoint {key} ({coord})')
for file in HERE.glob('*.md'):
    for target in re.findall(r'!?\[[^\]]*\]\(([^)]+)\)',file.read_text(encoding='utf-8')):
        if '://' in target:continue
        assert (file.parent/target.split('#')[0]).exists(),(file.name,target)
if problems:raise SystemExit('\n'.join(problems))
print('PASS: all rendered wire paths clear unrelated occupied pad centers by at least 9 SVG units; local Markdown links/images exist.')

manifest=json.loads((HERE/'generated/png-generation.json').read_text(encoding='utf-8'))
for name in ['generate-plan.py','render-board.py','export-png.mjs','../bench-harness.yml']:
    digest=hashlib.sha256((HERE/name).read_bytes()).hexdigest()
    if manifest['sources'].get(name) != digest:
        raise SystemExit(f"Stale L04 generation source: {name}; regenerate SVGs and PNGs")
for name in r['views']:
    for ext in ['svg','png']:
        path=HERE/'generated'/f'{name}.{ext}'
        digest=hashlib.sha256(path.read_bytes()).hexdigest()
        if manifest['files'].get(path.name) != digest:
            raise SystemExit(f"Stale/missing L04 PNG provenance: {path.name}; run export-png.mjs")
print('PASS: L04 generated documents/SVGs and PNG fingerprints are current; checks did not modify files.')
