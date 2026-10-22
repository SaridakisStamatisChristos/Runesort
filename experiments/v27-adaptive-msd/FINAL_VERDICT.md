# RUNE v27c — final qualification, 23 October 2026

**Integration gate: PASS. Comparative leadership is supported in 24 narrowly defined cells; universal sorting SOTA is not established.**

Measured source: `8b926fdcabc78a9d426e09eb03fd533b9af622c6`. Research qualification [run 39866998932](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39866998932): all eight jobs passed. Both standard source-commit CI runs passed too.

## Verified evidence

- Native x86 and ARM each passed all 34 CTest tests and all three leak-enabled ASan/UBSan variants. The local AVX-512 sanitizer build additionally exercised 149 protected-memory boundaries; the CI Intel entropy host exercised the SIMD reconstruction path.
- 10,890 complete exact-oracle observations retain 54,450 chronological timed sort calls and 10,890 checked warmups. Every expected cell, trial, seed, median and oracle flag was independently validated.
- All six downloaded artifact ZIP digests matched GitHub metadata. Recomputed summaries match the archived CI summaries byte for byte; raw CSV SHA-256 values are recorded in `evidence/v27c/ASSESSMENT.json`.
- All 198 candidate/v17 workload-size regression guards passed (fixed 5% bound). All four fixed x86 benefit cells passed (at least 10% improvement and 5/5 seed wins). Candidate/v17 paired ratios are 0.147652/0.145166 for sparse17 at 1M/4M and 0.223664/0.171665 for sparse20.
- 24 cells beat **every public comparator in that cell** by the fixed 5% margin, with 5/5 seed wins against each. Eighteen are on x86 and six on ARM; list and exact comparator names are in the assessment JSON. The worker-only MSD entry never contributes to a public superiority claim.

## SIMD-active Intel entropy result

Native CI Intel Xeon Platinum 8573C, GCC native release flags. Milliseconds below are medians of seed medians; acceptance uses seed-paired ratios, not ratios of these displayed medians.

| Workload | Keys | v27c | v17 | VQSort | x86-simd-sort |
|---|---:|---:|---:|---:|---:|
| sparse17 | 1,000,000 | 2.730 | 18.550 | 7.695 | 7.891 |
| sparse17 | 4,000,000 | 11.367 | 78.350 | 36.662 | 36.469 |
| sparse20 | 1,000,000 | 4.949 | 21.952 | 7.638 | 7.837 |
| sparse20 | 4,000,000 | 17.556 | 102.455 | 35.741 | 36.527 |

## Boundaries and negative evidence

Unrestricted random64 remains a substantial weakness. In the same Intel experiment at 4M, v27c was 280.642 ms against VQSort 35.445 ms; the median seed-paired latency ratio was 7.948. The unchanged fallback preserves v17 behavior rather than fixing that gap. Do not generalize compact-domain leadership to general-purpose sorting.

The x86 entropy host was Intel 8573C; x86 cardinality/ordered hosts were AMD EPYC 7763. ARM hosts were Neoverse-N2. Absolute milliseconds are never pooled across them. The prior scalar v27b source passed its independent AMD benefit and regression gates; this is useful cross-CPU evidence for shared scalar counting, not an AVX-512 performance claim.

v27a retained its **HOLD** result: sparse20/1M ratio 0.903123 missed the original 0.90 benefit threshold despite passing all regression guards. The threshold was not relaxed. v27b subsequently passed on its own run. Their assessments and immutable artifact manifests remain archived under `evidence/v27a` and `evidence/v27b`.

The scalar/SIMD mechanism pilot and reduced Intel runs are development evidence with different local link details. The supplementary pinned full-Intel attempt was incomplete and highly noisy; it is retained under `evidence/excluded-local-pinned` and excluded from every gate. None of these replaces current-source CI.

These are conventional counting, order-preserving bit extraction and SIMD techniques. No novelty, formal verification, production history, universal SOTA or blanket repository code-quality certification is claimed.

## Promotion

The focused main integration is [PR #32](https://github.com/SaridakisStamatisChristos/Runesort/pull/32). Its three sorting headers match this source byte for byte. The `runesort::sort` facade is a using-declaration; integration tests exercise native, scalar and portable paths while retaining the legacy v5 API and proprietary license.

The experiment source, frozen seeds, comparator pins and thresholds are in `PLAN.md`. Run `assess.py evidence/v27c 8b926fdcabc78a9d426e09eb03fd533b9af622c6 OUTPUT.json` to reproduce the final gate assessment.
