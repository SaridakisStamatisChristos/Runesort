#!/usr/bin/env python3
"""Independent fail-closed audit of published RUNE v14 full-gauntlet evidence."""
import csv
from collections import defaultdict
from pathlib import Path
from statistics import median
from math import isfinite

D = Path(__file__).resolve().parent / "evidence"
GROUPS = {
    "entropy": ["uniform64","random32","random16","extremes","sparse16",
                "sparse16alt","contig16","split16","sparse17","sparse20","sparse23","sparse24"],
    "cardinality": ["K16","K128","K1024","K4096","K8192","K16384","K32768",
                    "K40960","K65536","K73728","K81920","K131072"],
    "ordered": ["sorted","reversed","rotated","nearly","local","equal",
                "organpipe","sawtooth","alternating"],
}
BASE = {"RUNE_v14","RUNE_v13","RUNE_arm16","RUNE_v7","RUNE_v6",
        "RUNE_v5","VQSort","IPS4o_seq","pdqsort","spreadsort",
        "std_sort","std_stable_sort"}

def load(file):
    with (D/file).open(newline="") as f:
        return list(csv.DictReader(f))

def same_number(a,b):
    return abs(float(a)-float(b)) <= 0.0000021

def main():
    raw={}
    for arch,total in (("x86",3861),("arm64",3564)):
        rows=load(f"{arch}_raw.csv")
        assert len(rows)==total, (arch,len(rows),total)
        for z in rows:
            group,mode=z["group"],z["workload"]
            n,seed=int(z["n"]),int(z["seed"])
            name=z["algorithm"]
            assert group in GROUPS and mode in GROUPS[group]
            assert n in (100000,1000000,4000000)
            assert name in (BASE|{"CAFS_pdq"} if arch=="x86" else BASE)
            assert z["correct"]=="true" and int(z["repetitions"])==3
            lo,md,hi=map(float,(z["min_ms"],z["median_ms"],z["max_ms"]))
            assert all(map(isfinite,(lo,md,hi))) and 0<lo<=md<=hi
            key=(arch,group,mode,n,seed,name)
            assert key not in raw, f"duplicate {key}"
            raw[key]=md
    all_summ=load("paired_summary.csv")
    all_winner=load("winners.csv")
    assert len(all_summ)==2475 and len(all_winner)==198
    published={(z["arch"],z["group"],z["workload"],int(z["n"]),z["algorithm"]):z
               for z in all_summ}
    actual={}
    for (arch,group,mode,n,seed,name),ms in raw.items():
        actual.setdefault((arch,group,mode,n),{}).setdefault(name,{})[seed]=ms
    assert len(actual)==198
    by_winner={(z["arch"],z["group"],z["workload"],int(z["n"])):z for z in all_winner}
    assert len(by_winner)==198
    comparisons=0
    for key,v in actual.items():
        arch,group,mode,n=key
        expected=BASE|{"CAFS_pdq"} if arch=="x86" else BASE
        assert set(v)==expected, (key,expected-set(v))
        seeds=set(v["RUNE_v14"])
        assert len(seeds)==3
        mode_index=GROUPS[group].index(mode)
        expected_seeds={base + mode_index*131071 for base in (631003,631013,631021)}
        assert seeds==expected_seeds, (key,seeds,expected_seeds)
        rank=[]
        for name,values in v.items():
            assert set(values)==seeds
            ms=median(values.values())
            ratios=[values[seed]/v["RUNE_v14"][seed] for seed in seeds]
            row=published[(arch,group,mode,n,name)]
            assert same_number(row["median_seed_ms"],ms), (key,name,"median")
            assert same_number(row["paired_competitor_over_v14"],median(ratios))
            assert same_number(row["paired_min_ratio"],min(ratios))
            assert same_number(row["paired_max_ratio"],max(ratios))
            rank.append((name,ms))
            comparisons+=1
        rank.sort(key=lambda z:z[1])
        best=rank[0]
        winner=by_winner[key]
        assert winner["fastest_algorithm"]==best[0], key
        assert same_number(winner["fastest_ms"],best[1])
        assert same_number(winner["v14_ms"],median(v["RUNE_v14"].values()))
        assert same_number(winner["v14_time_over_fastest"],
                           median(v["RUNE_v14"].values())/best[1])
        tied=";".join(name for name,ms in rank if ms<=best[1]*1.02)
        assert tied==winner["within_2pct_of_fastest"],key
    print(f"AUDIT PASS: rows={len(raw)}, observations x86=3861 ARM64=3564; "
          f"algorithm-summaries={comparisons}, workload winners={len(actual)}")
    print("All 33 workload modes, sizes, seeds, algorithms, ratios, raw bounds, "
          "win/tie sets, exact-result flags verified against the archive.")

if __name__=="__main__":
    main()
