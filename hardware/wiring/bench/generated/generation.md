# W01 generation record

Generated from the authoritative source and renderer; no electrical validation is implied.

- Source SHA-256: `9fd5208733972fa7e8c556b8aabb85f99c0d8c655bb8cd3743ea9349efe6fc80`
- Renderer SHA-256: `729c4617566f788393f86395a12dd72861730cacc7a703e169d572d6d3db8099`
- Python: `3.12.10`
- WireViz: `0.4.1`
- PyYAML: `6.0.3`
- Python graphviz binding: `0.21`
- Graphviz executable: `dot - graphviz version 16.1.0 (20260904.0139)`

Command from repository root, with dependencies installed and `dot` on PATH:

```powershell
python hardware/wiring/bench/render.py
```

Each sectional view selects complete cable groups from `x-w01-views` in the source.
No pin or wire mapping is overridden. Colors follow the source's W01 convention;
the blank USB cable color is unassigned, not a ground indication.
