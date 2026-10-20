#!/usr/bin/env python3
"""Reproducible aggregation of the RUNE v14 full, architecture-separated gauntlet.

Input paths must contain 'arm64'/'aarch64' or 'x86'; the script refuses to pool CPUs.
Paired ratios are per-seed competitor / RUNE_v14 medians, then median across seeds.
Never interpret simple winner counts as statistical significance or SOTA.
"""
from __future__ import annotations

import argparse
from collections import defaultdict
import csv
from pathlib import Path
import statistics
import sys


def architecture(path: Path) -> str:
    name = str(path).lower()
    if "arm64" in name or "aarch64" in name or "ubuntu-24.04-arm" in name:
        return "arm64"
    if "x86" in name or "ubuntu-24.04" in name:
        return "x86"
    raise ValueError(f"cannot identify architecture (will not pool): {path}")


def read_rows(root: Path):
    seen = {}
    for path in sorted(root.rglob("*.csv")):
        with path.open(newline="") as f:
            reader = csv.DictReader(f)
            if not reader.fieldnames or "correct" not in reader.fieldnames:
                continue
            cpu = architecture(path)
            for row in reader:
                if row["correct"].lower() != "true":
                    raise ValueError(f"incorrect result in {path}: {row}")
                key = (cpu, row["group"], row["workload"], int(row["n"]),
                       int(row["seed"]), row["algorithm"])
                val = float(row["median_ms"])
                if val <= 0 or key in seen:
                    raise ValueError(f"duplicate/nonpositive result: {key}")
                seen[key] = val
    if not seen:
        raise ValueError("no raw gauntlet CSVs found")
    return seen


def aggregate(values):
    bycase = defaultdict(lambda: defaultdict(dict))
    for (cpu, group, mode, n, seed, algo), value in values.items():
        bycase[(cpu, group, mode, n)][algo][seed] = value
    summary = []
    for (cpu, group, mode, n), algorithms in sorted(bycase.items()):
        if "RUNE_v14" not in algorithms:
            raise ValueError(f"missing RUNE_v14 in {(cpu,group,mode,n)}")
        seeds = set(algorithms["RUNE_v14"])
        if len(seeds) != 3:
            raise ValueError(f"not three seeds in {(cpu,group,mode,n)}")
        for name, readings in algorithms.items():
            if set(readings) != seeds:
                raise ValueError(f"incomplete paired seeds {name} {cpu} {group} {mode} {n}")
            competitor = statistics.median(readings.values())
            ratios = [readings[s]/algorithms["RUNE_v14"][s] for s in seeds]
            summary.append({
                "architecture":cpu, "group":group, "workload":mode, "n":n,
                "algorithm":name, "input_seeds":len(seeds),
                "median_seed_ms":round(competitor, 5),
                "paired_median_competitor_over_v14":round(statistics.median(ratios), 6),
                "paired_min_ratio":round(min(ratios), 6),
                "paired_max_ratio":round(max(ratios), 6),
            })
    return summary


def table(rows, path: Path):
    path.parent.mkdir(parents=True,exist_ok=True)
    with path.open("w", newline="") as file:
        writer=csv.DictWriter(file, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def main() -> None:
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("raw_root",type=Path)
    parser.add_argument("--output",type=Path,default=Path("gauntlet_summary.csv"))
    args=parser.parse_args()
    results=read_rows(args.raw_root)
    summary=aggregate(results)
    table(summary,args.output)
    groups=defaultdict(list)
    for row in summary:
        if row["algorithm"] == "RUNE_v14":
            continue
        groups[(row["architecture"],row["n"])].append(row)
    for (cpu,n), rows in sorted(groups.items()):
        print(f"{cpu} n={n}: paired comparisons={len(rows)}; "
              f"RUNE_v14 faster by >2% in "
              f"{sum(x['paired_median_competitor_over_v14'] > 1.02 for x in rows)} cases, "
              f"slower by >2% in "
              f"{sum(x['paired_median_competitor_over_v14'] < 0.98 for x in rows)} cases")
    print(f"complete summary: {args.output}, rows={len(summary)} "
          f"from {len(results)} raw observations")
    print("2% is a descriptive tolerance, NOT a measured statistical confidence interval.")


if __name__ == "__main__":
    try:
        main()
    except (ValueError, OSError) as e:
        sys.exit(str(e))
