"""Check L01 against W01; optionally synchronize wires while preserving routes."""

import argparse
from collections import Counter
import json
from pathlib import Path

import yaml


ROOT = Path(__file__).resolve().parent
COLORS = {
    "BK": "black", "RD": "red", "OG": "orange", "YE": "yellow",
    "GN": "green", "BU": "blue", "VT": "violet", "WH": "white",
    "GY": "gray", "BN": "brown", "PK": "pink",
}
# Used native pins, verified against Wokwi's linked official board definition.
NATIVE_PINS = {
    "board-esp32-s3-devkitc-1": {
        "3V3.1", "GND.1", "5V", "4", "5", "6", "8", "9", "10", "11", "12",
    },
    "wokwi-pushbutton": {"1.l", "1.r", "2.l", "2.r"},
    "wokwi-resistor": {"1", "2"},
    "wokwi-text": set(),
}


def edge(a, b, color):
    return (*sorted((a, b)), color)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sync", action="store_true", help="Update connection endpoints/colors from W01; inspect routes afterward")
    args = parser.parse_args()
    diagram_path = ROOT / "diagram.json"
    diagram = json.loads(diagram_path.read_text(encoding="utf-8"))
    projection = json.loads((ROOT / "interface-map.json").read_text(encoding="utf-8"))
    source = yaml.safe_load((ROOT / projection["w01_source"]).read_text(encoding="utf-8"))
    endpoints = projection["endpoints"]
    parts = {part["id"]: part for part in diagram["parts"]}
    if len(parts) != len(diagram["parts"]):
        raise ValueError("Duplicate part IDs")
    if diagram["version"] != 1 or diagram["editor"] != "wokwi":
        raise ValueError("Unexpected diagram format")

    pin_sets = {}
    for part_id, part in parts.items():
        if part_id in projection["part_definitions"]:
            path = ROOT / projection["part_definitions"][part_id]
            definition = json.loads(path.read_text(encoding="utf-8"))
            pins = [pin for pin in definition["pins"] if pin]
            if len(pins) != len(set(pins)):
                raise ValueError(f"Duplicate custom pins: {path.name}")
            if part["type"] != "chip-" + path.name.removesuffix(".chip.json"):
                raise ValueError(f"Custom type/definition mismatch: {part_id}")
            if not path.with_suffix(".c").exists():
                raise ValueError(f"Missing online placeholder stub: {path.name}")
            pin_sets[part_id] = set(pins)
        else:
            if part["type"] not in NATIVE_PINS:
                raise ValueError(f"Unreviewed native type: {part['type']}")
            pin_sets[part_id] = NATIVE_PINS[part["type"]]

    for physical, virtual in endpoints.items():
        part_id, pin = virtual.split(":")
        if part_id not in pin_sets or pin not in pin_sets[part_id]:
            raise ValueError(f"Unknown mapped virtual endpoint: {virtual}")
        owner, terminal = physical.split(".")
        if terminal not in source["connectors"][owner]["pins"]:
            raise ValueError(f"Unknown W01 terminal: {physical}")
        if owner == "J1" and virtual.startswith("J1:") and pin.isdigit():
            j1 = source["connectors"]["J1"]
            label = j1["pinlabels"][j1["pins"].index(terminal)]
            if not label.startswith(f"GPIO{pin} /"):
                raise ValueError(f"GPIO mismatch for {physical}: {label} versus {virtual}")

    expected = []
    seen_omissions = set()
    for group in source["connections"]:
        (a, ap), (bundle, nums), (b, bp) = [next(iter(item.items())) for item in group]
        if not len(ap) == len(nums) == len(bp):
            raise ValueError(f"Unequal W01 conductor lists: {bundle}")
        if bundle in projection["omitted_bundles"]:
            seen_omissions.add(bundle)
            continue
        for x, number, y in zip(ap, nums, bp):
            expected.append((endpoints[f"{a}.{x}"], endpoints[f"{b}.{y}"], COLORS[source["cables"][bundle]["colors"][number - 1]]))
    if seen_omissions != {"W_USB"} or set(projection["omitted_bundles"]) != {"W_USB"}:
        raise ValueError("Only the documented whole USB cable may be omitted")
    expected.extend(tuple(connection) for connection in projection["internal_connections"])
    if projection["internal_connections"] != [["R1:2", "C1:OUT", "pink"]]:
        raise ValueError("Unexpected internal RC wiring; review component topology")
    if parts["R1"]["attrs"].get("value") != "470":
        raise ValueError("R1 no longer matches the selected 470 ohm filter")

    if args.sync:
        routes = {(a, b, color): route for a, b, color, route in diagram["connections"]}
        diagram["connections"] = [[a, b, color, routes.get((a, b, color), [])] for a, b, color in expected]
        diagram_path.write_text(json.dumps(diagram, indent=2) + "\n", encoding="utf-8", newline="\n")

    for a, b, color, route in diagram["connections"]:
        if not isinstance(route, list):
            raise ValueError("Wire route must be a list")
        for virtual in (a, b):
            part_id, pin = virtual.split(":")
            if part_id not in pin_sets or pin not in pin_sets[part_id]:
                raise ValueError(f"Unknown wire endpoint: {virtual}")
    actual = Counter(edge(a, b, color) for a, b, color, _ in diagram["connections"])
    target = Counter(edge(*connection) for connection in expected)
    if actual != target:
        raise ValueError(f"W01 mismatch. Missing: {list((target - actual).elements())}; extra: {list((actual - target).elements())}")
    print(f"PASS: {len(parts)} parts, {len(diagram['connections'])} wires; W01 endpoints/colors, unique IDs, custom/native pins and RC topology. USB cable is annotated.")


if __name__ == "__main__":
    main()
