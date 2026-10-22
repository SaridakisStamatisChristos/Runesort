# v27 performance evidence

The source under evaluation is
[`8b926fdcabc78a9d426e09eb03fd533b9af622c6`](https://github.com/SaridakisStamatisChristos/Runesort/commit/8b926fdcabc78a9d426e09eb03fd533b9af622c6).
The three sorting headers in this integration are byte-identical to that source;
`SOURCE_PROVENANCE.json` records their SHA-256 values. The public namespace uses
a `using` declaration and adds no runtime selection overhead.

The complete qualification harness, fixed gates, raw chronological measurements,
CPU/compiler provenance and analysis live in
[PR #31](https://github.com/SaridakisStamatisChristos/Runesort/pull/31).
Earlier sources and their results cannot qualify this revision. Hosted runner
CPUs may differ between groups; compare seed-paired latencies within each host
experiment, and do not pool absolute milliseconds across hosts.

To reproduce the external comparison, check out the research source and run
`.github/workflows/v27-qualification.yml`. To independently audit downloaded
results, arrange the six `rune-v27-<runner>-<group>` directories under one path
and run:

```sh
python3 experiments/v27-adaptive-msd/assess.py RESULTS_DIRECTORY \
  8b926fdcabc78a9d426e09eb03fd533b9af622c6 ASSESSMENT.json
```

The script verifies every expected workload, size, seed, comparator, oracle flag,
trial and median; it reproduces all six summaries byte for byte before computing
regression, benefit and tightly scoped comparative-winning gates. Performance
gates remain distinct from correctness CI. Broad random64 SOTA is not claimed.

## Qualified result

All eight research qualification jobs passed for the pinned source, including
34 CTest checks per architecture and three leak-enabled sanitizer configurations.
All 198 regression guards and four primary benefit gates passed. Strict comparative
leadership was observed in 24 specified cells; `ASSESSMENT.json` records every
one and the raw data hashes. The earlier scalar AMD revision also passed its
independent regression and benefit gates; no absolute host times are pooled.

[Immutable verdict and complete raw CSV archive](https://github.com/SaridakisStamatisChristos/Runesort/blob/20870417c5f0be9f01347b91fdbe03ba1eb1dd29/experiments/v27-adaptive-msd/FINAL_VERDICT.md).

The 4M random64 result remains materially behind VQSort (280.64 vs 35.45 ms on
the recorded Intel host). This integration improves compact domains and retains
the old general fallback. It does not resolve that separate high-entropy gap.
The research archive preserves the initial failed target, earlier assessments,
development pilots and an excluded incomplete noisy local attempt.
