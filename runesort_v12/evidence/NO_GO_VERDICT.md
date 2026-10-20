# RUNE v12 — page pre-touch experiment: verified NO-GO

## Decision
**NO-GO: do not promote or merge v12.** Adding one volatile scratch write per nominal 4096-byte stride does not resolve v11's memory-first-touch regressions and produces major losses in tested regimes.

## Reproducibility

- Source pinned at tested commit `9daf8f17277a9ed77e9e7560f32a320c6e48da2b`; this evidence-only commit does not alter algorithm source.
- [Final native ARM64/x86 benchmarks and sanitizers (4/4 passed)](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39427215107).
- [Full CMake build and test](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39427215110) passed; 25/25 CTest tests each architecture in native matrix; ASan/UBSan passed both, x86 -mno-bmi2 passed.
- GCC 13.3 C++17, Ubuntu 24.04, ARM64 Neoverse-N2 and x86 AMD EPYC 9V74 virtualized GitHub runners.
- Same-source variants v10 (zero-initialized scratch), frozen v11 (uninitialized scratch), v12 (uninitialized + volatile page stride stores). Changes isolated to compacted-bit radix.
- n=262144,524288,1000000,2000000,4000000; eight workload families; five new seeds, warmup plus four measured trials per seed; randomized variant order; copied identical inputs; each sort exactly checked against std::sort.
- **1200 of 1200 benchmark output rows were exact and correct**, 600 each architecture. Raw data and all 80 comparisons are preserved.
- Times are medians of seed-level medians in milliseconds; percentage is the median of five **within-seed v10/v12** (or v11/v12) time ratios minus one. Never pool CPU architectures.

## Sparse-bit workloads

| Architecture | N | mask | v10 ms | v11 ms | v12 ms | paired v12 vs v10 | paired v12 vs v11 |
|---|---:|---|---:|---:|---:|---:|---:|
| x86_epyc9v74 | 1000000 | 20 | 10.742 | 11.691 | 12.016 | -9.40% | -3.13% |
| x86_epyc9v74 | 1000000 | 22 | 13.049 | 13.433 | 13.924 | -3.40% | -0.32% |
| x86_epyc9v74 | 1000000 | 23 | 14.787 | 15.581 | 16.834 | -11.77% | -5.40% |
| x86_epyc9v74 | 4000000 | 20 | 52.035 | 49.418 | 52.093 | -0.53% | -5.12% |
| x86_epyc9v74 | 4000000 | 22 | 62.514 | 59.763 | 62.559 | +0.43% | -4.58% |
| x86_epyc9v74 | 4000000 | 23 | 75.457 | 73.666 | 75.710 | +0.34% | -3.01% |
| arm64_neoverse_n2 | 1000000 | 20 | 39.037 | 39.027 | 44.830 | -12.92% | -13.08% |
| arm64_neoverse_n2 | 1000000 | 22 | 43.402 | 43.325 | 43.390 | -0.06% | -0.20% |
| arm64_neoverse_n2 | 1000000 | 23 | 46.970 | 46.872 | 46.694 | +0.38% | +0.05% |
| arm64_neoverse_n2 | 4000000 | 20 | 156.263 | 156.242 | 179.217 | -12.73% | -12.78% |
| arm64_neoverse_n2 | 4000000 | 22 | 174.529 | 174.445 | 174.132 | +0.14% | +0.19% |
| arm64_neoverse_n2 | 4000000 | 23 | 188.797 | 188.787 | 188.519 | +0.19% | +0.06% |

## Interpretation and scope

- Volatile pre-touch was specifically used to prevent compiler dead-store elimination of page-touch instrumentation.
- The tested x86 1M sparse20 and sparse23 cases regress by roughly 9–12% versus v10; ARM 1M and 4M sparse20 regress by roughly 13%. Other regimes show little effect or mixed small changes. This refutes the proposed straightforward pre-touch optimization in these regimes.
- The test does not establish a causal explanation of OS page faults or cache placement: it lacks hardware performance counters, page-fault attribution, and controlled physical machines.
- Both uninitialized and pre-touched buffers remain safe in the tested programs because each histogram-prefix partition covers [0,n) and every radix pass overwrites all destinations before reading the buffer.
- No general SOTA or novel-algorithm claim is justified; the stronger existing high-cardinality bottleneck remains a distinct research problem.

## Evidence files

- [x86 raw CSV](x86_epyc9v74_raw.csv)
- [ARM raw CSV](arm64_neoverse_n2_raw.csv)
- [Full 80-case native summary](summary_80_cases.csv)

**Evidence state:** correctness empirically validated for the executed suite; performance improvement hypothesis **REFUTED on tested distributions**. Keep PR draft and do not merge. Preserve v10 and v11 unchanged.
