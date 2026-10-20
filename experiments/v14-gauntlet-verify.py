#!/usr/bin/env python3
"""Fail-closed completeness/correctness audit for the v14 full benchmark."""
import csv
import math
import platform
import sys
from collections import defaultdict
from pathlib import Path

GROUPS = {
    "entropy": ["uniform64", "random32", "random16", "extremes",
                "sparse16", "sparse16alt", "contig16", "split16",
                "sparse17", "sparse20", "sparse23", "sparse24"],
    "cardinality": ["K16", "K128", "K1024", "K4096", "K8192", "K16384",
                    "K32768", "K40960", "K65536", "K73728", "K81920", "K131072"],
    "ordered": ["sorted", "reversed", "rotated", "nearly", "local", "equal",
                "organpipe", "sawtooth", "alternating"],
}
BASE = {"RUNE_v14", "RUNE_v13", "RUNE_arm16", "RUNE_v7", "RUNE_v6",
        "RUNE_v5", "VQSort", "IPS4o_seq", "pdqsort", "spreadsort",
        "std_sort", "std_stable_sort"}

def main():
    if len(sys.argv) != 4:
        sys.exit("usage: verify.py csv group n")
    source, group, n = Path(sys.argv[1]), sys.argv[2], int(sys.argv[3])
    assert group in GROUPS and n in (100000, 1000000, 4000000)
    with source.open(newline="") as stream:
        rows = list(csv.DictReader(stream))
    assert rows, f"no data {source}"
    sets = defaultdict(set)
    observed_algs = set()
    for row in rows:
        assert row["group"] == group and row["workload"] in GROUPS[group], row
        assert int(row["n"]) == n and int(row["repetitions"]) == 3, row
        assert row["correct"].lower() == "true", row
        median = float(row["median_ms"])
        lo, hi = float(row["min_ms"]), float(row["max_ms"])
        assert all(math.isfinite(v) for v in (median, lo, hi))
        assert 0 < lo <= median <= hi, row
        label = row["algorithm"]
        assert label in BASE | {"CAFS_pdq"}, row
        observed_algs.add(label)
        # Each workload input seed must include all algorithms exactly once.
        key = (row["workload"], row["seed"])
        assert label not in sets[key], f"duplicate result: {key} {label}"
        sets[key].add(label)
    assert BASE <= observed_algs, f"missing comparator {BASE-observed_algs}"
    assert observed_algs in (BASE, BASE | {"CAFS_pdq"}), observed_algs
    if platform.machine().lower() in ("aarch64", "arm64"):
        assert "CAFS_pdq" not in observed_algs, "CAFS is x86 AVX2/BMI-only"
    assert len(sets) == len(GROUPS[group])*3, (len(sets), group)
    assert all(v == observed_algs for v in sets.values()), "incomplete algorithms"
    assert len(rows) == len(GROUPS[group])*3*len(observed_algs)
    print(f"VERIFIED {source}: rows={len(rows)}, inputs={len(sets)}, "
          f"algorithms={len(observed_algs)}, n={n}, "
          f"all correct=true and complete, architecture={platform.machine()}",
          file=sys.stderr)
    return 0

if __name__ == "__main__":
    sys.exit(main())
