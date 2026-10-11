"""Validate L03 against W01-checked L01, or refresh wires preserving routes."""

import argparse
from collections import Counter
import json
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parent


def edge(a, b, color):
    return (*sorted((a, b)), color)


def projected_connections(functional, mapping):
    raw = next(key for key, value in mapping['signal_aliases'].items() if value == 'R1:1')
    out = next(key for key, value in mapping['signal_aliases'].items() if value == 'R1:2')
    connections = []
    for a, b, color, _ in functional['connections']:
        if a.startswith('RAILS:'):
            a = 'BB1:' + mapping['power_taps'][b]
        elif b.startswith('RAILS:'):
            b = 'BB1:' + mapping['power_taps'][a]
        if a == 'FTP:B':
            b = f'BB1:{raw}.f'
        if a == 'R1:2' and b == 'J3:A0':
            a = f'BB1:{out}.f'
        if a == 'R1:2' and b == 'C1:OUT':
            a = f'BB1:{out}.g'
        connections.append((a, b, color))
    connections.extend(tuple(wire[:3]) for wire in mapping['helper_connections'])
    return connections


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sync', action='store_true')
    args = parser.parse_args()
    subprocess.run([sys.executable, str(ROOT.parent / 'check.py')], check=True)
    functional = json.loads((ROOT.parent / 'diagram.json').read_text(encoding='utf-8'))
    path = ROOT / 'diagram.json'
    diagram = json.loads(path.read_text(encoding='utf-8'))
    mapping = json.loads((ROOT / 'layout-map.json').read_text(encoding='utf-8'))
    parts = {part['id']: part for part in diagram['parts']}
    assert len(parts) == len(diagram['parts']), 'Duplicate part IDs'
    assert diagram['version'] == 1 and diagram['editor'] == 'wokwi'
    assert parts['BB1']['type'] == mapping['breadboard_type'] == 'wokwi-breadboard'
    assert mapping['rail_aliases'] == {'tp': 'RAILS:3V3', 'tn': 'RAILS:GND', 'bp': 'RAILS:5V', 'bn': 'RAILS:GND'}
    assert len(mapping['signal_aliases']) == 2
    assert set(mapping['signal_aliases'].values()) == {'R1:1', 'R1:2'}
    assert all(re.fullmatch(r'(?:[1-9]|[1-5][0-9]|6[0-3])b', key) for key in mapping['signal_aliases'])
    assert parts['R1']['attrs']['value'] == '470'
    source_parts = {part['id']: part for part in functional['parts']}
    electrical = {key for key, part in parts.items() if part['type'] != 'wokwi-text'}
    assert electrical == {'BB1', 'J1', 'J2', 'J3', 'LS1', 'FTP', 'R1', 'C1', 'S1'}
    layout_types = {'J3': 'chip-pcv-ads1115-header', 'LS1': 'chip-pcv-level-shifter-header'}
    for key in electrical - {'BB1'}:
        assert parts[key]['type'] == layout_types.get(key, source_parts[key]['type']), f'Changed representation: {key}'
    # Header order is component-side, left to right after rotating 270 degrees.
    for name, pins in {
        'pcv-ads1115-header': ['V', 'G', 'SCL', 'SDA', 'ADDR', 'ALERT', 'A0', 'A1', 'A2', 'A3'] + [''] * 10,
        'pcv-level-shifter-header': ['LV1', 'LV2', 'LV', 'GND_LV', 'LV3', 'LV4', 'HV4', 'HV3', 'GND_HV', 'HV', 'HV2', 'HV1'],
    }.items():
        definition = json.loads((ROOT.parent / 'chips' / f'{name}.chip.json').read_text())
        assert definition['pins'] == pins, f'Header order differs from component reference: {name}'
    assert parts['J3']['rotate'] == parts['LS1']['rotate'] == 270
    expected = projected_connections(functional, mapping)
    if args.sync:
        routes = {(a, b, color): route for a, b, color, route in diagram['connections']}
        diagram['connections'] = [[a, b, color, routes.get((a, b, color), [])] for a, b, color in expected]
        path.write_text(json.dumps(diagram, indent=2) + '\n', encoding='utf-8', newline='\n')
    actual = Counter(edge(a, b, color) for a, b, color, _ in diagram['connections'])
    assert actual == Counter(edge(*wire) for wire in expected), 'Projected wiring differs from L01/W01'

    def normalized(endpoint):
        if not endpoint.startswith('BB1:'):
            assert endpoint in {pin for wire in functional['connections'] for pin in wire[:2]}
            return endpoint
        pin = endpoint.split(':')[1]
        rail = re.fullmatch(r'(tp|tn|bp|bn)\.([1-9]|1[0-9]|2[0-5])', pin)
        if rail:
            # Use only the first 25 taps: no reliance on any centre rail break.
            return mapping['rail_aliases'][rail[1]]
        strip = re.fullmatch(r'([1-9][0-9]*b)\.([f-j])', pin)
        assert strip and strip[1] in mapping['signal_aliases'], f'Unmapped breadboard strip: {pin}'
        return mapping['signal_aliases'][strip[1]]

    projected = []
    joins = []
    occupied = Counter()
    for a, b, color, route in diagram['connections']:
        assert isinstance(route, list)
        for endpoint in (a, b):
            if endpoint.startswith('BB1:'):
                occupied[endpoint] += 1
        x, y = normalized(a), normalized(b)
        if x == y:
            assert (x, color) in {('RAILS:GND', 'black'), ('R1:1', 'brown'), ('R1:2', 'pink')}
            joins.append((x, color))
        else:
            projected.append(edge(x, y, color))
    assert all(count == 1 for count in occupied.values()), 'Multiple jumpers occupy one breadboard hole'
    assert Counter(joins) == Counter({('RAILS:GND', 'black'): 1, ('R1:1', 'brown'): 1, ('R1:2', 'pink'): 1})
    assert Counter(projected) == Counter(edge(a, b, color) for a, b, color, _ in functional['connections']), 'Breadboard groups alter intended circuit'
    print(f'PASS L03: {len(parts)} parts, {len(diagram["connections"])} wires; distinct supply/raw/OUT groups, ground bridge, one jumper per hole and W01 connectivity/colors.')


if __name__ == '__main__':
    main()
