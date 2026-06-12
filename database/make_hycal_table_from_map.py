#!/usr/bin/env python3
"""Generate the prad2 HyCal geometry table from the prad2 reconstruction map.

The PRad-II reconstruction (prad2det) resolves cluster centers against
hycal_map.json, so for the prad2/x17 configurations the simulation must
build the calorimeter from the SAME map — this script converts it into the
table format HyCalModule reads (576 lead-glass rows then 1152 PbWO4 rows of
"size_x size_y x y", mm).

The legacy table (hycal_module_shuffled.dat, used by prad/drad) carries the
lead-glass ring transposed and offset by ~0.3 mm relative to the map; PbWO4
positions are identical in both.

Usage:
  ./make_hycal_table_from_map.py [hycal_map.json] [output.dat]

Defaults: $PRAD2_DATABASE_DIR/hycal_map.json (or the prad2 toolkit install)
          -> hycal_module_prad2.dat next to this script.
"""

import json
import os
import sys


def main():
    here = os.path.dirname(os.path.abspath(__file__))

    map_path = sys.argv[1] if len(sys.argv) > 1 else None
    if map_path is None:
        candidates = [
            os.path.join(os.environ.get("PRAD2_DATABASE_DIR", ""), "hycal_map.json"),
            os.path.expanduser("~/Apps/prad2/share/prad2evviewer/database/hycal_map.json"),
        ]
        map_path = next((c for c in candidates if c and os.path.isfile(c)), None)
        if map_path is None:
            sys.exit("hycal_map.json not found; pass it as the first argument")

    out_path = sys.argv[2] if len(sys.argv) > 2 else os.path.join(here, "hycal_module_prad2.dat")

    with open(map_path) as f:
        modules = json.load(f)

    lg = [m for m in modules if m.get("t") == "PbGlass"]
    pwo = [m for m in modules if m.get("t") == "PbWO4"]

    if len(lg) != 576 or len(pwo) != 1152:
        sys.exit(f"unexpected module counts: PbGlass={len(lg)} PbWO4={len(pwo)} "
                 "(expected 576 / 1152)")

    with open(out_path, "w") as f:
        for group in (lg, pwo):
            for m in group:
                g = m["geo"]
                f.write(f"{g['sx']:10.4f} {g['sy']:10.4f} {g['x']:12.4f} {g['y']:12.4f}\n")

    print(f"wrote {out_path}: {len(lg)} PbGlass + {len(pwo)} PbWO4 from {map_path}")


if __name__ == "__main__":
    main()
