#!/usr/bin/env python3
"""Aggregate architecture-specific RUNE comparisons without pooling CPUs.

Usage:
  python3 experiments/summarize_crossarch.py path/to/100k.csv path/to/1m.csv --out results.csv

Input: CSVs from experiments/v7-crossarch-competition.cpp.
Each row is a per-seed median of four measured repetitions after one warmup.
"""
import argparse
import csv
from collections import defaultdict
from pathlib import Path
from statistics import median

COLUMNS = ("RUNE_v7", "RUNE_v6", "RUNE_v5", "VQSort",
           "IPS4o_seq", "pdqsort", "spreadsort", "std_sort")
SEEDS = {271828, 314159, 777777}


def summarize(files):
    observations = defaultdict(dict)
    for path in files:
        with Path(path).open(newline="") as f:
            for r in csv.DictReader(f):
                if r.get("correct", "").lower() != "true":
                    raise ValueError(f"Wrong or missing correctness flag in {path}: {r}")
                n, seed = int(r["n"]), int(r["seed"])
                name, algo = r["workload"], r["algorithm"]
                key = (n, name, algo)
                if seed in observations[key]:
                    raise ValueError(f"Repeated seed: {key} {seed}")
                observations[key][seed] = float(r["median_ms"])
    groups = defaultdict(dict)
    for (n, name, algo), by_seed in observations.items():
        if set(by_seed) != SEEDS:
            raise ValueError(f"Incomplete seeds for {n},{name},{algo}: {set(by_seed)}")
        groups[(n, name)][algo] = median(by_seed.values())
    result = []
    for (n, name), data in sorted(groups.items()):
        if set(data) != set(COLUMNS):
            raise ValueError(f"Missing algorithms for {n}/{name}: {set(COLUMNS)-set(data)}")
        winner = min(COLUMNS, key=lambda a: data[a])
        result.append({"n": n, "workload": name, **{a: round(data[a],5) for a in COLUMNS},
                       "winner": winner,
                       "v6_over_v7": round(data["RUNE_v6"]/data["RUNE_v7"],4),
                       "vq_over_v7": round(data["VQSort"]/data["RUNE_v7"],4)})
    return result


def main():
    p = argparse.ArgumentParser()
    p.add_argument("input", nargs="+")
    p.add_argument("--out", required=True)
    args = p.parse_args()
    rows = summarize(args.input)
    if not rows:
        raise SystemExit("No benchmark data")
    with Path(args.out).open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0]))
        w.writeheader()
        w.writerows(rows)
    for row in rows:
        if row["n"] == 1000000:
            print(f'{row["workload"]:>12}: winner={row["winner"]:<12} v6/v7={row["v6_over_v7"]:.3f}x, VQ/v7={row["vq_over_v7"]:.3f}x')


if __name__ == "__main__":
    main()
