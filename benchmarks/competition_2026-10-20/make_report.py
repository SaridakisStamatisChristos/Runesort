#!/usr/bin/env python3
"""Aggregate local reruns without modifying the preserved historical measurements."""
import csv
import json
import statistics
import sys
from pathlib import Path
folder=Path(sys.argv[1]) if len(sys.argv)>1 else Path(__file__).resolve().parent/"reproduced"
files=sorted(folder.glob("results_seed_*.csv"))
if len(files)!=4: raise ValueError(f"Expected four seed files, found {len(files)}")
groups={}
for path in files:
    with path.open(newline="") as fp:
        for row in csv.DictReader(fp):
            groups.setdefault((row["n"], row["mode"]),[]).append(row)
cols=["v5_ms","v4_ms","pdqsort_ms","spreadsort_ms","std_sort_ms"]
summary=[]
for (n,mode),rows in sorted(groups.items()):
    if len(rows)!=4: raise ValueError(f"Incomplete seeds: {n}/{mode}")
    out={"n":int(n),"mode":mode,"seeds":len(rows)}
    for col in cols: out[col]=round(statistics.median(float(r[col]) for r in rows),4)
    out["fastest"]=min(cols,key=lambda k:out[k])
    summary.append(out)
with (folder/"summary.csv").open("w",newline="") as fp:
    writer=csv.DictWriter(fp,fieldnames=list(summary[0]))
    writer.writeheader();writer.writerows(summary)
(folder/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
print(f"Wrote {len(summary)} cases to {folder}/summary.csv")
