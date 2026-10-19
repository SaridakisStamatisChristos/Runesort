#!/usr/bin/env python3
import csv
import json
import statistics
from pathlib import Path
root=Path(__file__).resolve().parent
records={}
for f in sorted(root.glob("release_seed_*.csv")):
    with f.open() as fp:
        for row in csv.DictReader(fp):
            records.setdefault(row["mode"],[]).append(row)
summary=[]
for mode,rows in sorted(records.items()):
    if len(rows)!=4: raise ValueError(f"Incomplete results for {mode}: {len(rows)} seeds")
    d={"mode":mode}
    for key in ("v5_ms","v4_ms","pdqsort_ms","spreadsort_ms","std_sort_ms"):
        d[key]=round(statistics.median(float(r[key]) for r in rows),4)
    d["v4_over_v5"]=round(d["v4_ms"]/d["v5_ms"],3)
    d["per_seed_ratios"]=[round(float(r["v4_ms"])/float(r["v5_ms"]),3) for r in rows]
    summary.append(d)
(root/"release_multiseed_summary.json").write_text(json.dumps(summary,indent=2)+"\n")
for row in summary: print(f"{row['mode']}: {row['v4_over_v5']}x")
