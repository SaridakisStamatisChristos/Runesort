# RUNE v10 — Scratch-buffer initialization experiment

**Research status: empirically promising, small and workload-dependent gains; not a confirmed universal speedup or new algorithm.**

## Mechanism and evidence provenance
- Frozen RUNE v7 variant with only two O(n) allocation sites changed: fixed_radix and dense_radix used to value-initialize their scratch uint64 arrays to zero; v10 default-initializes each via RAII unique_ptr with uninitialized element values.
- Safety invariant: per-pass histogram prefix ranges partition [0,n), and stable scatter fills each slot exactly once before that scratch array is used as next pass's input. Compacted-domain/frequency/ordering paths are entirely unchanged.
- [Native GitHub CI benchmark run](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39416704023): ARM64 Neoverse-N2 and x86-64 AMD EPYC 7763, **both completed successfully**; all sort outputs checked against std::sort. Tested implementation commit `487b260447c995b685578e936fdcbf0bc9c3ff5c`.
- [Independent baseline build/test](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39416703955): succeeded. A separate ASan+UBSan native workflow validates direct radix scratch paths.
- GCC C++17 `-O3 -march=native -DNDEBUG`, one sorting thread, 5 fixed independent seeds, one warmup + 4 measured rounds/seed, randomized variant order, same generated input and exact std::sort oracle.
- N=1M and N=4M; 10 distributions with random64, random32, sparse16, K40960, K73728, K81920, K90112, K131072, local inversions, near-sorted noise.
- Times are medians of per-seed medians, **ms**; input copy/verification excluded from sort-only interval. CI runners are virtualized, not pinned, and small changes should be treated as tentative until further replications.

## ARM64 Neoverse-N2 — N=1,000,000

| Workload | v7 ms | v10 ms | v7/v10 |
|---|---:|---:|---:|
| uniform64 | 16.642 | 16.395 | 1.015× |
| random32 | 12.170 | 11.716 | 1.039× |
| sparse16 | 18.363 | 18.408 | 0.998× |
| K40960 | 9.589 | 9.601 | 0.999× |
| K73728 | 12.394 | 12.356 | 1.003× |
| K81920 | 15.496 | 15.207 | 1.019× |
| K90112 | 15.344 | 15.020 | 1.022× |
| K131072 | 15.089 | 14.459 | 1.044× |
| local | 1.808 | 1.799 | 1.005× |
| nearly | 5.259 | 5.295 | 0.993× |

## ARM64 Neoverse-N2 — N=4,000,000

| Workload | v7 ms | v10 ms | v7/v10 |
|---|---:|---:|---:|
| uniform64 | 87.204 | 84.276 | 1.035× |
| random32 | 55.785 | 53.171 | 1.049× |
| sparse16 | 68.940 | 68.769 | 1.002× |
| K40960 | 30.593 | 30.647 | 0.998× |
| K73728 | 36.771 | 36.669 | 1.003× |
| K81920 | 77.602 | 75.100 | 1.033× |
| K90112 | 74.419 | 71.969 | 1.034× |
| K131072 | 72.674 | 70.033 | 1.038× |
| local | 7.531 | 7.608 | 0.990× |
| nearly | 22.074 | 22.151 | 0.996× |

## x86-64 AMD EPYC 7763 — N=1,000,000

| Workload | v7 ms | v10 ms | v7/v10 |
|---|---:|---:|---:|
| uniform64 | 30.075 | 29.706 | 1.012× |
| random32 | 20.638 | 20.503 | 1.007× |
| sparse16 | 6.603 | 6.770 | 0.975× |
| K40960 | 8.644 | 8.322 | 1.039× |
| K73728 | 12.707 | 12.315 | 1.032× |
| K81920 | 29.185 | 28.040 | 1.041× |
| K90112 | 29.329 | 28.642 | 1.024× |
| K131072 | 36.277 | 35.501 | 1.022× |
| local | 1.814 | 1.832 | 0.990× |
| nearly | 7.678 | 7.636 | 1.005× |

## x86-64 AMD EPYC 7763 — N=4,000,000

| Workload | v7 ms | v10 ms | v7/v10 |
|---|---:|---:|---:|
| uniform64 | 123.310 | 122.734 | 1.005× |
| random32 | 84.919 | 85.203 | 0.997× |
| sparse16 | 24.772 | 25.300 | 0.979× |
| K40960 | 26.252 | 25.264 | 1.039× |
| K73728 | 33.456 | 32.029 | 1.045× |
| K81920 | 95.802 | 95.227 | 1.006× |
| K90112 | 97.405 | 96.309 | 1.011× |
| K131072 | 105.826 | 105.235 | 1.006× |
| local | 7.419 | 7.474 | 0.993× |
| nearly | 32.494 | 32.389 | 1.003× |

## Interpretation and negative evidence

- ARM64: 8 of 20 workload/size combinations improved by at least 2% and 0 worsened by at least 2% in median-of-seed-medians.
- x86: 7 of 20 cases improved by at least 2%, 2 worsened by at least 2%.
- The improvement is strongest where v7 invokes radix sorting: ARM64 at N=4M random32 ~5%, random64 ~3.5% and K81920 ~3.3%; x86 at N=4M K40960/K73728 ~4%.
- Some apparently unaffected paths show small regressions, notably x86 sparse16 (~2% at both scales), which does not touch either changed scratch allocation. This points to environmental noise / placement changes; **do not claim v10 improves everything**.
- The optimization cannot solve v7's broader high-cardinality disadvantage versus VQSort on particular CPUs. No VQSort/CAFS experiment was run for v10. Prior versions v8 and v9 failed on unrelated mechanisms.
- No substantive novelty claim: avoiding redundant vector initialization is established C++ performance engineering, not a newly invented sorting algorithm.

## Decision and next proof obligations

**Conditional GO for additional research only.** V10 is a small, correctness-tested engineering optimization with some repeatable same-run gains; evidence does not yet justify merging or advertising overall SOTA. Keep PR draft until memory sanitizer CI passes and results replicate on more dedicated CPUs and seeds. Preserve original v7 and earlier negative experiments.

### Raw artifacts
- [ARM64 measurements](arm64_neoverse_n2_raw.csv), [ARM64 medians](arm64_neoverse_n2_summary.csv)
- [x86-64 measurements](x86_amd_epyc_7763_raw.csv), [x86 medians](x86_amd_epyc_7763_summary.csv)
