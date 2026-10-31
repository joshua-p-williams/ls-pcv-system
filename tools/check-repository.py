"""Check public documentation and generated wiring using W01's Python dependencies.

Run from any directory: python tools/check-repository.py
This is document validation, not firmware or physical hardware testing.
"""

from collections import Counter
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import subprocess
import sys
from urllib.parse import unquote
import xml.etree.ElementTree as ET

import yaml

ROOT = Path(__file__).resolve().parents[1]


def markdown_anchors(text):
    """GitHub-style slugs for the ordinary ATX headings used in this repository."""
    text = re.sub(r"```.*?```", "", text, flags=re.S)
    used = Counter()
    anchors = set()
    for heading in re.findall(r"^#{1,6}\s+(.+?)\s*#*\s*$", text, re.M):
        heading = re.sub(r"\[([^]]+)\]\([^)]*\)", r"\1", heading)
        slug = re.sub(r"[^\w\- ]", "", heading.lower()).replace(" ", "-")
        anchors.add(slug if used[slug] == 0 else f"{slug}-{used[slug]}")
        used[slug] += 1
    anchors.update(re.findall(r'\bid=["\']([^"\']+)["\']', text))
    return anchors


def main():
    os.chdir(ROOT)
    paths = set(subprocess.check_output(
        ["git", "ls-files", "--cached", "--others", "--exclude-standard"], text=True
    ).splitlines())
    errors = []
    documents = {name: (ROOT / name).read_text(encoding="utf-8")
                 for name in paths if name.endswith(".md") and (ROOT / name).is_file()}
    anchors = {(ROOT / name).resolve(): markdown_anchors(text)
               for name, text in documents.items()}
    for name, text in documents.items():
        for target in re.findall(r'!?\[[^\]]*\]\(([^\s)]+)(?:\s+"[^"]*")?\)', text):
            if re.match(r"[a-zA-Z][a-zA-Z0-9+.-]*:", target):
                continue
            path_text, _, fragment = unquote(target.strip("<>")).partition("#")
            path = ((ROOT / name).parent / path_text).resolve() if path_text else (ROOT / name).resolve()
            if not path.exists():
                errors.append(f"{name}: missing link {target}")
                continue
            if not path.is_relative_to(ROOT):
                errors.append(f"{name}: local link leaves repository: {target}")
                continue
            current = ROOT
            for segment in path.relative_to(ROOT).parts:
                if segment not in {p.name for p in current.iterdir()}:
                    errors.append(f"{name}: filename case mismatch: {target}")
                    break
                current /= segment
            if fragment and path in anchors and fragment not in anchors[path]:
                errors.append(f"{name}: missing heading {target}")
    for name in paths:
        path = ROOT / name
        if not path.is_file():
            continue
        if name.startswith("_staging/") or path.suffix.lower() == ".fcbak":
            errors.append(f"Private or backup artifact is tracked: {name}")
        if path.suffix == ".json":
            json_text = path.read_text(encoding="utf-8")
            # VS Code's extensions.json is JSONC, so its documented full-line
            # comments are valid there even though json.loads rejects them.
            if path.name == "extensions.json" and path.parent.name == ".vscode":
                json_text = re.sub(r"(?m)^\s*//.*(?:\n|$)", "", json_text)
            json.loads(json_text)
        if path.suffix == ".svg":
            ET.parse(path)

    folder = ROOT / "hardware/wiring/bench"
    spec = importlib.util.spec_from_file_location("w01_renderer", folder / "render.py")
    renderer = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(renderer)
    source = yaml.safe_load((folder / "bench-harness.yml").read_text(encoding="utf-8"))
    renderer.validate(source)
    for view in source["x-w01-views"].values():
        subset = renderer.selected_view(source, view["cables"])
        if any(group not in source["connections"] for group in subset["connections"]):
            errors.append("W01 sectional view overrides source connectivity")
    if (folder / "generated/connections.md").read_text(encoding="utf-8") != renderer.connection_table(source):
        errors.append("W01 connection schedule is stale; run hardware/wiring/bench/render.py")
    generation = (folder / "generated/generation.md").read_text(encoding="utf-8")
    for label, name in [("Source", "bench-harness.yml"), ("Renderer", "render.py")]:
        digest = hashlib.sha256((folder / name).read_bytes()).hexdigest()
        if f"{label} SHA-256: `{digest}`" not in generation:
            errors.append(f"W01 {label.lower()} fingerprint differs; regenerate all views")
    if errors:
        raise SystemExit("\n".join(sorted(errors)))
    # L03 first validates L01 against W01, then its rail/strip projection.
    subprocess.run([sys.executable, "simulation/wokwi/breadboard/check.py"], check=True)
    subprocess.run([sys.executable, "hardware/wiring/bench/protoboard/check-layout.py"], check=True)
    print(f"PASS: {len(documents)} Markdown files: local links, anchors and case; JSON/SVG syntax; "
          "W01 generation fingerprints, schedule and derived connections; L01/L03 wiring; L04 layout and outputs. "
          "No firmware or hardware tests performed.")


if __name__ == "__main__":
    main()
