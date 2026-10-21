#!/usr/bin/env python3
"""Validate the predeclared full matrix and emit seed-paired descriptive metrics."""
import argparse
import csv
import math
import statistics
from pathlib import Path

SEEDS = (20301001, 20301011, 20301021, 20301031, 20301041)
SIZES = (65536, 999999, 1000000, 4000000)
MODES = ('uniform64', 'random32', 'K85000', 'sorted', 'alternating',
         'reverse', 'sparse16', 'almostsorted', 'sortedprefix', 'entropy_prefix')
ALGORITHMS = ('RUNE17_full', 'RUNE25_full_network', 'RUNE26_full_nibble',
              'RUNE26_full_stdleaf', 'VQSort_current', 'IPS4o_seq_current')
CANDIDATES = ('RUNE26_full_nibble', 'RUNE26_full_stdleaf')
BASELINES = ('RUNE17_full', 'RUNE25_full_network', 'RUNE26_full_stdleaf',
             'VQSort_current', 'IPS4o_seq_current')


def summarize(paths, isa, output):
    rows = {}
    capabilities = set()
    for path in paths:
        with Path(path).open(newline='') as stream:
            for row in csv.DictReader(stream):
                if row['simd_available'] not in ('0', '1'):
                    raise ValueError('invalid SIMD capability')
                capabilities.add(row['simd_available'])
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
                for n in SIZES for seed in SEEDS for algorithm in ALGORITHMS}
    if set(rows) != expected:
        raise ValueError(f'incomplete/unexpected matrix: missing={len(expected-set(rows))}, '
                         f'extra={len(set(rows)-expected)}')
    if len(capabilities) != 1 or (isa == 'aarch64' and capabilities != {'0'}):
        raise ValueError('inconsistent SIMD capability')
    capability = capabilities.pop()
    fields = ['simd_available', 'isa', 'mode', 'n', 'candidate', 'candidate_median_ms']
    for base in BASELINES:
        fields += [f'{base}_median_ms', f'ratio_to_{base}', f'wins_vs_{base}']
    with Path(output).open('w', newline='') as stream:
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        for mode in MODES:
            for n in SIZES:
                for candidate in CANDIDATES:
                    times = [rows[mode, n, seed, candidate] for seed in SEEDS]
                    result = dict(simd_available=capability, isa=isa, mode=mode, n=n, candidate=candidate,
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
