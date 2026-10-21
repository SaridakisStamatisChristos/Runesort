#!/usr/bin/env python3
"""Verify matrix identities and emit architecture-scoped seed-paired summary."""
import csv,math,statistics,sys
from pathlib import Path
from collections import defaultdict
def run(isa,group,out,paths):
    rows={}
    for path in paths:
        with Path(path).open(newline="") as f:
            for r in csv.DictReader(f):
                key=(r["workload"],int(r["n"]),int(r["seed"]),r["algorithm"])
                if key in rows:raise ValueError("duplicate "+str(key))
                trial=[float(r["trial"+str(i)+"_ms"]) for i in range(3)]
                if any(not math.isfinite(t) or t<=0 for t in trial):raise ValueError("invalid trials")
                if abs(trial[1]-float(r["median_ms"]))>1e-8:raise ValueError("mismatched trial median")
                if r["group"]!=group or r["correct"]!="true":raise ValueError("invalid row")
                rows[key]=(trial[1],r["simd_available"])
    by=defaultdict(dict)
    for (mode,n,seed,algo),(ms,simd) in rows.items():by[mode,n,algo][seed]=ms
    summary=[]
    for (mode,n,algo),seedms in sorted(by.items()):
        baseline=by.get((mode,n,"RUNE_v24"))
        if not baseline or len(baseline)!=3 or set(baseline)!=set(seedms):raise ValueError("invalid paired seeds")
        ratios=[seedms[s]/baseline[s] for s in sorted(seedms)]
        summary.append(dict(isa=isa,group=group,mode=mode,n=n,algorithm=algo,
          category=("kernel" if algo.endswith("_WORKER") else "full"),
          median_ms=statistics.median(seedms.values()),
          median_seed_paired_ratio_to_v24=statistics.median(ratios),
          seed_wins_over_v24=sum(seedms[s]<baseline[s] for s in baseline)))
    with Path(out).open("w",newline="") as f:
        w=csv.DictWriter(f,fieldnames=list(summary[0]))
        w.writeheader();w.writerows(summary)
    print(f"{isa} {group}: {len(rows)} oracle-verified rows, {len(summary)} summary cells")
    for n in (100000,1000000,4000000):
        subset=[r for r in summary if r["mode"]=="uniform64" and r["n"]==n and r["category"]=="full"]
        for r in sorted(subset,key=lambda r:r["median_ms"])[:7]:
            print(f'  random64 {n}: {r["algorithm"]} {r["median_ms"]:.3f} ms; candidate/baseline {r["median_seed_paired_ratio_to_v24"]:.3f}')
if __name__=="__main__":
    if len(sys.argv)<6:raise SystemExit("usage: summarize.py ISA GROUP OUTPUT CSV1 CSV2 CSV3")
    run(sys.argv[1],sys.argv[2],sys.argv[3],sys.argv[4:])
