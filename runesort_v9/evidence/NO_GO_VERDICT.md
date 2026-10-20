# RUNE v9 exact 131K-key reconstruction: independent benchmark NO-GO

**Decision: REFUTED performance hypothesis. Do not merge.** The extra exact table is correct on tested inputs, but it slows the complete sorter relative to frozen RUNE v7 on key workloads.

## Execution provenance
- [Successful GitHub build and correctness workflow](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39398070399)
- [Successful independent ARM64/x86 fast comparison](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39398070552)
- Executed v9 commit `ebdacab67972314d10afb9da21ee3ef2b5c5effc` on GitHub `ubuntu-24.04` (AMD EPYC 9V45 x86-64) and `ubuntu-24.04-arm` (Neoverse-N2 ARM64).
- GCC `-O3 -march=native -DNDEBUG -std=c++17`, N=1,000,000, full-width uint64 arrays, 5 **fresh** independent seeds, 1 warmup and 4 measured repetitions per seed.
- 3 actual implementations were timed on identical arrays per workload: candidate v9, frozen v7, and C++ `std::sort`. Every run was checked against std::sort reference. Sort time includes internal hash table/pair allocations and excludes input generation/working-copy/reference verification.
- These numbers are medians of per-seed medians. Virtual-machine scheduling noise remains possible; repeated large regressions on two architectures are much less likely to be noise. CAFS and VQSort were not included in this **fast falsification** stage because candidate v9 already failed to outperform frozen v7.

## X86-64 AMD EPYC 9V45 — milliseconds (lower is better)

| Workload | v9 | v7 | v7/v9 |
|---|---:|---:|---:|
| sparse16 | 4.322 | 4.370 | 1.011x |
| uniform64 | 9.475 | 9.362 | 0.988x |
| K73728 | 9.648 | 9.953 | 1.032x |
| K81920 | 12.441 | 11.405 | 0.917x |
| K90112 | 13.078 | 11.159 | 0.853x |
| K98304 | 14.170 | 11.375 | 0.803x |
| K114688 | 15.450 | 11.449 | 0.741x |
| K131072 | 11.389 | 11.141 | 0.978x |
| K147456 | 10.789 | 10.636 | 0.986x |
| K200000 | 10.451 | 10.576 | 1.012x |

## ARM64 Neoverse-N2 — milliseconds (lower is better)

| Workload | v9 | v7 | v7/v9 |
|---|---:|---:|---:|
| sparse16 | 4.294 | 4.319 | 1.006x |
| uniform64 | 14.868 | 14.649 | 0.985x |
| K73728 | 11.964 | 12.007 | 1.004x |
| K81920 | 14.170 | 14.206 | 1.003x |
| K90112 | 14.748 | 14.046 | 0.952x |
| K98304 | 15.303 | 13.901 | 0.908x |
| K114688 | 16.428 | 13.782 | 0.839x |
| K131072 | 13.736 | 13.537 | 0.986x |
| K147456 | 13.727 | 13.465 | 0.981x |
| K200000 | 13.361 | 13.293 | 0.995x |

## Why the mechanism loses

The enlarged exact tier uses 524,288 slots holding 8-byte keys, 4-byte counters, and 1-byte occupancy flags, plus up to 131,072 key/frequency pairs (several more MiB). It must initialize memory, hash and probe all n keys, materialize all distinct pairs, sort them, and reconstruct n outputs. At K90K–115K on tested CPUs, these costs exceed v7's general fallback. Near K131K the pilot often rejects and routes to the unchanged fallback, explaining the near-parity result there.

Successful isolated sixth-tier completion is therefore **not sufficient** to show better end-to-end dispatch. The raw local single-tier timing was hypothesis-generation, not comparative evidence.

## Correctness and limitations

The CMake suite and direct benchmark correctness checks passed. Failed sixth-tier reconstruction leaves the array untouched for the tested capacity and collision cases. This is **empirical**, not a formally verified proof.

Do not merge or make a global SOTA/novelty claim. RUNE v7 remains the stronger research candidate; the next useful direction is to improve its *existing general fallback* or optimize key/pair sorting and allocation, with exact same-run benchmarks. No change to `main` or `LICENSE`.

## Raw files
- [x86 five-seed measurements](x86_epyc9v45_raw.csv), [x86 summary](x86_epyc9v45_summary.csv)
- [ARM five-seed measurements](arm_neoverse_n2_raw.csv), [ARM summary](arm_neoverse_n2_summary.csv)
