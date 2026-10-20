# RUNE v17 — Completed cross-architecture native qualification

**Verdict: CONDITIONAL GO for the isolated experiment; HOLD MERGE; NO global SOTA claim.**

Exact implementation tested: [`7c87921c30217d4317a964fed07debcc16b2f183`](https://github.com/SaridakisStamatisChristos/Runesort/commit/7c87921c30217d4317a964fed07debcc16b2f183).
[GitHub Actions qualification](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39472827645): **4/4 jobs passed** (native x86+ARM, sanitized x86+ARM), including CMake/CTest, ASan/UBSan and x86 no-BMI2. Both separate [main CI push](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39472827347) and [PR CI](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39472895838) passed. All **280/280 measured algorithm-sort rows correct**; direct adversarial tests also passed.

## Method and evidence boundaries

Same-run randomized pair order, frozen v16c compared to v17, same exact input, full-array `std::sort` oracle, five input seeds, five measured sorts following one warmup, GCC 13.3 C++17 `-O3 -march=native -DNDEBUG`, N=1M and 4M, 7 distributions. Report medians of five seed-level trial medians and median of five *paired baseline/candidate ratios* separately; ratios need not equal ratio of displayed medians. Hardware is virtualized GitHub-hosted **x86 AMD EPYC 7763** or **ARM Neoverse-N2**, 4 vCPUs each, **not dedicated hosts**.

## Representative observations (N=4 million, milliseconds)

| CPU | Input | Frozen v16c | v17 | Median paired improvement | Winning seeds |
|---|---|---:|---:|---:|---:|
| x86 | uniform64 | 106.719 | 106.559 | 0.19% | 4/5 |
| x86 | random32 | 69.305 | 69.348 | -0.12% | 1/5 |
| x86 | sparse20 | 51.041 | 49.480 | 3.15% | 4/5 |
| x86 | K81920 | 33.922 | 33.208 | 1.83% | 5/5 |
| x86 | K85000 | 81.750 | 80.364 | 1.83% | 5/5 |
| x86 | sorted | 3.811 | 3.806 | 0.23% | 4/5 |
| x86 | nearly | 12.615 | 12.613 | -0.14% | 1/5 |
| arm64 | uniform64 | 70.704 | 69.125 | 4.12% | 5/5 |
| arm64 | random32 | 50.057 | 49.861 | 0.69% | 3/5 |
| arm64 | sparse20 | 76.614 | 76.830 | -0.68% | 1/5 |
| arm64 | K81920 | 36.051 | 36.129 | 0.22% | 3/5 |
| arm64 | K85000 | 71.518 | 69.773 | 3.28% | 5/5 |
| arm64 | sorted | 3.733 | 3.718 | -0.31% | 2/5 |
| arm64 | nearly | 17.116 | 16.795 | 0.38% | 3/5 |

Full 1M and 4M summaries: [paired_summary.csv](paired_summary.csv).
Exact raw data: [x86_raw.csv](x86_raw.csv), [arm64_raw.csv](arm64_raw.csv). Provenance: [provenance.json](provenance.json).

## Mechanism attribution and negative controls

The **only** implementation difference is value-initialization removal for `fixed_radix<8>`; other algorithms and dispatch paths are identical. ARM 4M uniform64 improves by about 4.12% paired, 5/5 seeds, consistent with avoiding an O(n) zero-fill; x86 4M uniform64 improves only ~0.19%, below practical noise thresholds. K85000 also improves on these hosts, but neither architecture supports a universal or reproducible speedup claim. On ARM 4M sparse20 the candidate is ~0.68% slower; on x86 1M sparse20 ~0.84% slower. Many other modes show mixed near-zero differences.

**Crucial attribution caveat:** Some v17/v16c variations occur on modes that do **not** execute changed fixed8 worker (notably K81920 when exact frequency reconstruction succeeds, and sparse20). They cannot be causal evidence of scratch allocation gains. Placement, virtual-runner noise, instruction-cache variation and randomized timing may explain those apparent wins/losses. We therefore **do not** promote the candidate based on them.

Previous [v10 PR #9](https://github.com/SaridakisStamatisChristos/Runesort/pull/9) already tested the initialization technique against v7, with mixed effects. v17 is a narrowly scoped engineering backport; no novelty or generalized SOTA inference. v15 six-pass was refuted and stays excluded.

## Decision

Keep as an unmerged conditional research candidate. For release promotion: repeat the high-entropy experiment on controlled same-hardware runs, measure allocation/memory bandwidth with profiling, then test against pinned Highway VQSort and IPS4o on the same hosts. x86 full-width random remains a substantial gap. No code rewrite or additional merge is justified by these data.
