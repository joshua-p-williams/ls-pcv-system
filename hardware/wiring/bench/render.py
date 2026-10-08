"""Render W01 and assembly views from one WireViz source; no connectivity overrides."""

from __future__ import annotations

import argparse
import copy
import hashlib
from importlib.metadata import version
from pathlib import Path
import platform
import subprocess

import yaml
from wireviz import wireviz


HERE = Path(__file__).resolve().parent


def selected_view(source: dict, cable_names: list[str]) -> dict:
    """Select complete connection groups without changing endpoints or wire IDs."""
    unknown = set(cable_names) - source["cables"].keys()
    if unknown:
        raise ValueError(f"Unknown cables in view: {sorted(unknown)}")
    result = copy.deepcopy(source)
    result["connections"] = [
        group for group in result["connections"] if next(iter(group[1])) in cable_names
    ]
    connector_names = {
        next(iter(group[end])) for group in result["connections"] for end in (0, 2)
    }
    result["connectors"] = {
        key: value for key, value in result["connectors"].items() if key in connector_names
    }
    result["cables"] = {
        key: value for key, value in result["cables"].items() if key in cable_names
    }
    return result


def validate(source: dict) -> None:
    """Validate the explicit three-part connection form used by this harness."""
    terminals = source["connectors"]
    cables = source["cables"]
    used_wires: set[tuple[str, int]] = set()
    for name, connector in terminals.items():
        pins = connector["pins"]
        if len(pins) != len(set(pins)) or len(pins) != len(connector["pinlabels"]):
            raise ValueError(f"Invalid pin/label list in {name}")
    for name, cable in cables.items():
        if len(cable["colors"]) != cable["wirecount"]:
            raise ValueError(f"Missing conductor colors in {name}")
        if any(code and code not in source["x-w01-color-convention"] for code in cable["colors"]):
            raise ValueError(f"Undefined color convention in {name}")
    for group in source["connections"]:
        if len(group) != 3 or any(len(part) != 1 for part in group):
            raise ValueError("W01 requires explicit endpoint / cable / endpoint groups")
        (left, left_pins), (cable, wires), (right, right_pins) = [
            next(iter(part.items())) for part in group
        ]
        if not all(isinstance(values, list) for values in (left_pins, wires, right_pins)):
            raise ValueError("Use explicit lists for W01 pins and wires")
        if not len(left_pins) == len(wires) == len(right_pins):
            raise ValueError(f"Mismatched endpoint lengths in {cable}")
        for name, pins in ((left, left_pins), (right, right_pins)):
            if name not in terminals or not set(pins) <= set(terminals[name]["pins"]):
                raise ValueError(f"Unknown terminal in {name}: {pins}")
        for wire in wires:
            if not isinstance(wire, int) or not 1 <= wire <= cables[cable]["wirecount"]:
                raise ValueError(f"Invalid wire {cable}/{wire}")
            if (cable, wire) in used_wires:
                raise ValueError(f"Wire {cable}/{wire} defined twice")
            used_wires.add((cable, wire))
    expected = {
        (name, number)
        for name, cable in cables.items()
        for number in range(1, cable["wirecount"] + 1)
    }
    if used_wires != expected:
        raise ValueError(f"Unconnected cable conductors: {expected - used_wires}")
    for view in source["x-w01-views"].values():
        selected_view(source, view["cables"])


def markdown_cell(value: object) -> str:
    return str(value).replace("|", "\\|").replace("\n", "; ")


def connection_table(source: dict) -> str:
    lines = [
        "# W01 connection schedule",
        "",
        "Generated from [bench-harness.yml](../bench-harness.yml); do not edit this table.",
        "**Draft: not assembly-ready.** `TBD` names are functional requirements, not terminal IDs.",
        "`W_USB` represents a complete USB cable, not one conductor. Other wire numbers identify",
        "conductors within a documentation bundle, not connector cavities. Colors specify added bench wiring,",
        "not purchased pigtail colors or verified terminal functions. Label and record substitutions.",
        "",
        "| Bundle / wire | Net or function | Planned color | Endpoint A | Endpoint B | Evidence / dependency |",
        "| --- | --- | --- | --- | --- | --- |",
    ]
    for group in source["connections"]:
        (left, lp), (cable, wires), (right, rp) = [next(iter(p.items())) for p in group]
        spec = source["cables"][cable]
        for a, wire, b in zip(lp, wires, rp):
            cells = [
                f"{cable} / {wire}",
                spec["wirelabels"][wire - 1],
                (f"{spec['colors'][wire - 1]} / "
                 f"{source['x-w01-color-convention'][spec['colors'][wire - 1]]['name']}"
                 if spec['colors'][wire - 1] else "Unassigned (see note)"),
                f"{left}.{a}",
                f"{right}.{b}",
                spec["notes"],
            ]
            lines.append("| " + " | ".join(markdown_cell(c) for c in cells) + " |")
    lines.extend([
        "", "## Wire color convention", "",
        "Project convention for W01 additions, not a universal electrical color standard.",
        "Use labels to distinguish buses sharing clock/data colors. Color alone does not identify a net.",
        "Display VCC uses orange for the selected 3.3 V supply; the nominal 5 V branch uses red.",
        "The factory USB cable has no specified jacket or internal-conductor color.",
        "Blank-color lines render neutrally; they are not instructions to use ground wire.", "",
        "| Code | Color | Use |", "| --- | --- | --- |",
    ])
    for code, color in source["x-w01-color-convention"].items():
        lines.append(f"| {code} | {color['name']} | {color['use']} |")
    lines.extend([
        "", "## Proposed controller assignments", "",
        "Derived from J1 in the same source. L coordinates refer to P01's front-view left row.",
        "These are design proposals under Q14, not measured pin functions.", "",
        "| J1 location | GPIO / function |", "| --- | --- |",
    ])
    for pin, label in zip(source["connectors"]["J1"]["pins"], source["connectors"]["J1"]["pinlabels"]):
        lines.append(f"| {markdown_cell(pin)} | {markdown_cell(label)} |")
    return "\n".join(lines) + "\n"


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, default=HERE / "generated")
    parser.add_argument("--preview-dir", type=Path, help="Optional private PNG preview directory")
    args = parser.parse_args()
    source_path = HERE / "bench-harness.yml"
    source_bytes = source_path.read_bytes()
    source = yaml.safe_load(source_bytes)
    validate(source)
    args.output_dir.mkdir(parents=True, exist_ok=True)
    if args.preview_dir:
        args.preview_dir.mkdir(parents=True, exist_ok=True)

    views = {"bench-harness": copy.deepcopy(source)}
    for name, config in source["x-w01-views"].items():
        view = selected_view(source, config["cables"])
        view["tweak"]["append"][0] = view["tweak"]["append"][0].replace(
            "W01 - DRAFT / NOT ASSEMBLY READY",
            f"W01 - {config['title']} - DRAFT / NOT ASSEMBLY READY",
        )
        views[f"bench-harness-{name}"] = view
    for name, view in views.items():
        wireviz.parse(copy.deepcopy(view), output_formats=("svg",),
                      output_dir=args.output_dir, output_name=name)
        # Graphviz emits platform line endings; keep public text artifacts LF.
        svg_path = args.output_dir / f"{name}.svg"
        svg_text = svg_path.read_text(encoding="utf-8")
        svg_path.write_text(svg_text, encoding="utf-8", newline="\n")
        if args.preview_dir:
            wireviz.parse(copy.deepcopy(view), output_formats=("png",),
                          output_dir=args.preview_dir, output_name=name)
    (args.output_dir / "connections.md").write_text(connection_table(source), encoding="utf-8", newline="\n")
    graphviz_version = subprocess.run(["dot", "-V"], capture_output=True, text=True, check=True)
    dot = (graphviz_version.stdout + graphviz_version.stderr).strip()
    record = [
        "# W01 generation record", "",
        "Generated from the authoritative source and renderer; no electrical validation is implied.", "",
        f"- Source SHA-256: `{hashlib.sha256(source_bytes).hexdigest()}`",
        f"- Renderer SHA-256: `{hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}`",
        f"- Python: `{platform.python_version()}`",
        f"- WireViz: `{version('wireviz')}`",
        f"- PyYAML: `{version('PyYAML')}`",
        f"- Python graphviz binding: `{version('graphviz')}`",
        f"- Graphviz executable: `{dot}`", "",
        "Command from repository root, with dependencies installed and `dot` on PATH:", "",
        "```powershell", "python hardware/wiring/bench/render.py", "```", "",
        "Each sectional view selects complete cable groups from `x-w01-views` in the source.",
        "No pin or wire mapping is overridden. Colors follow the source's W01 convention;",
        "the blank USB cable color is unassigned, not a ground indication.", "",
    ]
    (args.output_dir / "generation.md").write_text("\n".join(record), encoding="utf-8", newline="\n")
    print(f"Validated and rendered {len(views)} views plus connection schedule and generation record.")


if __name__ == "__main__":
    main()
