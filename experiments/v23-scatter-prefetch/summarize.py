#!/usr/bin/env python3
"""Validate the predeclared full matrix and emit seed-paired descriptive metrics."""
import argparse
import csv
import math
import statistics
from pathlib import Path

SEEDS = (20281037, 20281043, 20281049, 20281051, 20281057)
MODES = ('uniform64', 'random32', 'K85000', 'sorted', 'alternating')
ALGORITHMS = ('RUNE17_full', 'RUNE17_fixed8', 'RUNE22_radix11',
              'RUNE22_radix16', 'VQSort', 'RUNE23_ablation0',
              'RUNE23_prefetch32', 'RUNE23_prefetch128', 'IPS4o_seq')
CANDIDATES = ALGORITHMS[6:8]
BASELINES = ('RUNE23_ablation0', 'RUNE22_radix16', 'RUNE17_fixed8', 'VQSort')


def summarize(paths, isa, output):
    rows = {}
    for path in paths:
        with Path(path).open(newline='') as stream:
            for row in csv.DictReader(stream):
                key = row['mode'], int(row['n']), int(row['seed']), row['algorithm']
                if key in rows:
                    raise ValueError(f'duplicate observation: {key}')
                if row['correct'] != 'true':
                    raise ValueError(f'incorrect observation: {key}')
                trials = [float(row[f'trial{i}_ms']) for i in range(3)]
                if not all(math.isfinite(t) and t > 0 for t in trials):
                    raise ValueError(f'invalid timing: {key}')
                for field, expected in [('median_ms', statistics.median(trials)),
                                        ('min_ms', min(trials)), ('max_ms', max(trials))]:
                    if float(row[field]) != expected:
                        raise ValueError(f'{field} does not match raw trials: {key}')
                rows[key] = statistics.median(trials)
    expected = {(mode, n, seed, algorithm) for mode in MODES
                for n in (1000000, 4000000) for seed in SEEDS for algorithm in ALGORITHMS}
    if set(rows) != expected:
        raise ValueError(f'incomplete/unexpected matrix: missing={len(expected-set(rows))}, '
                         f'extra={len(set(rows)-expected)}')
    fields = ['isa', 'mode', 'n', 'candidate', 'candidate_median_ms']
    for base in BASELINES:
        fields += [f'{base}_median_ms', f'ratio_to_{base}', f'wins_vs_{base}']
    with Path(output).open('w', newline='') as stream:
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        for mode in MODES:
            for n in (1000000, 4000000):
                for candidate in CANDIDATES:
                    times = [rows[mode, n, seed, candidate] for seed in SEEDS]
                    result = dict(isa=isa, mode=mode, n=n, candidate=candidate,
                                  candidate_median_ms=statistics.median(times))
                    for base in BASELINES:
                        ref = [rows[mode, n, seed, base] for seed in SEEDS]
                        result[f'{base}_median_ms'] = statistics.median(ref)
                        result[f'ratio_to_{base}'] = statistics.median(a/b for a, b in zip(times, ref))
                        result[f'wins_vs_{base}'] = sum(a < b for a, b in zip(times, ref))
                    writer.writerow(result)
    print(f'{isa}: validated {len(rows)} observations / {len(rows)*3} measured sorts, '
          f'plus {len(rows)} verified warmups; wrote {output}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--isa', required=True)
    parser.add_argument('--output', required=True)
    parser.add_argument('csvs', nargs='+')
    args = parser.parse_args()
    summarize(args.csvs, args.isa, args.output)
