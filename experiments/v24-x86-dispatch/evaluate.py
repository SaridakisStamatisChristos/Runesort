#!/usr/bin/env python3
"""Apply the predeclared latency gates to two already-validated summary CSVs."""
import argparse
import csv
import json
import math
from pathlib import Path

from summarize import MODES, SIZES


def evaluate(paths):
    cells = {}
    for path in paths:
        with Path(path).open(newline='') as stream:
            for row in csv.DictReader(stream):
                if row['candidate'] != 'RUNE24_full_prefetch128':
                    continue
                key = row['isa'], row['mode'], int(row['n'])
                if key in cells:
                    raise ValueError(f'duplicate summary cell {key}')
                cells[key] = row
    expected = {(isa, mode, n) for isa in ('x86_64', 'aarch64')
                for mode in MODES for n in SIZES}
    if cells.keys() != expected:
        raise ValueError('incomplete/unexpected summary matrix')

    def check(key, baseline, limit, wins=None):
        row = cells[key]
        ratio = float(row[f'ratio_to_{baseline}'])
        count = int(row[f'wins_vs_{baseline}'])
        if not math.isfinite(ratio) or ratio <= 0 or not 0 <= count <= 5:
            raise ValueError(f'invalid summary values {key}')
        return dict(isa=key[0], mode=key[1], n=key[2], baseline=baseline,
                    paired_ratio=ratio, seed_wins=count, ratio_limit=limit,
                    required_wins=wins,
                    passed=ratio <= limit and (wins is None or count >= wins))

    targets = {('x86_64', 'uniform64', n) for n in (1000000, 4000000)}
    groups = {
        'end_to_end_benefit': [check(k, 'RUNE17_full', .95, 5) for k in sorted(targets)],
        'hint_contribution': [check(('x86_64', 'uniform64', 4000000),
                                    'RUNE24_full_ablation0', .95, 5)],
        'regression_guard': [check(k, 'RUNE17_full', 1.05)
                             for k in sorted(expected - targets)],
        'pinned_vqsort_superiority': [check(k, 'VQSort', .95, 5) for k in sorted(targets)],
    }
    return {name: dict(passed=all(c['passed'] for c in checks),
                       failures=[c for c in checks if not c['passed']], checks=checks)
            for name, checks in groups.items()}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('summaries', nargs=2)
    parser.add_argument('--output', required=True)
    args = parser.parse_args()
    verdict = evaluate(args.summaries)
    Path(args.output).write_text(json.dumps(verdict, indent=2) + '\n')
    for name, result in verdict.items():
        print(f"{name}: {'PASS' if result['passed'] else 'FAIL'}; "
              f"{len(result['failures'])}/{len(result['checks'])} failed cells")
