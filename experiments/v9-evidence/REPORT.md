# Research ablation — 131K-key reconstruction (conditional findings)

**Status:** Exact-output CI-validated experiment. **NO-GO for unconditional RUNE v9 promotion.** Frozen RUNE v7 sorter remains unchanged.

## Evidence and methodology

- [Completed native ARM64 and x86 GitHub Actions run](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39384630712), both jobs passed.
- The benchmark source is [v9-capacity-ablation.cpp](../v9-capacity-ablation.cpp); source pinned at commit `e78ef620443feebbc50b7c7cf31a438be435ae13`.
- x86 CPU AMD EPYC 7763; ARM CPU Neoverse-N2; Ubuntu 24.04, GCC 13.3, `-O3 -march=native -DNDEBUG -std=c++17`.
- N=200K and 1M; 13 workload families; 4 seeds; 3 measured repetitions per seed after a warmup. Rotated comparator order, equal input per algorithm; all results verified against `std::sort`. Times include all allocation and entire rejected-attempt+v7-fallback overhead, excluding input generation/copy and correctness verification.
- Two exact hash-frequency variants have 131,072-key limit, bounded 32-probe rejection, and either 262,144 or 524,288 slots. Memory initialized per invocation approximately 3.25 MB vs 6.5 MB, plus unique key pairs. On rejection the original input is unchanged.
- The table below shows **median of the four per-seed medians**, in milliseconds; successful attempt counts span 4 seeds × 3 measured repetitions (12 total). A candidate's own rerun may have a warmup result not reflected in acceptance totals.

## x86 AMD EPYC 7763 (1 million keys)

| Distribution | v7 ms | 256K slots ms | 512K slots ms | Successful / 12 (A,B) |
|---|---:|---:|---:|---:|
| K73728 | 12.53 | 13.23 | 13.27 | 12, 12 |
| K81920 | 24.68 | 14.98 | 14.28 | 12, 12 |
| K90112 | 24.48 | 16.25 | 15.01 | 12, 12 |
| K98304 | 24.57 | 17.43 | 15.78 | 12, 12 |
| K114688 | 25.42 | 19.54 | 17.83 | 12, 12 |
| K131072 | 25.35 | 25.19 | 20.11 | 6, 12 |
| K147456 | 25.51 | 30.21 | 29.93 | 0, 0 |
| random64 | 26.88 | 29.71 | 29.33 | 0, 0 |
| sparse16 | 5.90 | 10.59 | 10.55 | 12, 12 |

At K81,920 and K90,112, the 512K-slot table is 24.68→14.28 ms and 24.48→15.01 ms; nevertheless this is a *target-specific* result, not VQSort superiority. At K131,072 the 256K-slot table rejects about half of the measured repeats because of bounded collisions; the 512K-slot table accepts all 12. Above K131K or on random64 it fails and adds overhead; on sparse16 it is almost twice as slow as frozen v7. At N200K, pdqsort is fastest on eight cases.

## ARM64 Neoverse-N2 (1 million keys)

| Distribution | v7 ms | 256K slots ms | 512K slots ms | Successful / 12 (A,B) |
|---|---:|---:|---:|---:|
| K73728 | 11.45 | 11.64 | 12.34 | 12, 12 |
| K81920 | 13.68 | 12.65 | 12.93 | 12, 12 |
| K90112 | 13.92 | 13.46 | 13.47 | 12, 12 |
| K98304 | 13.79 | 14.26 | 13.99 | 12, 12 |
| K114688 | 13.53 | 15.77 | 15.09 | 12, 12 |
| K131072 | 13.41 | 17.23 | 16.95 | 6, 12 |
| K147456 | 13.41 | 16.76 | 16.50 | 0, 0 |
| random64 | 14.42 | 15.55 | 15.60 | 0, 0 |
| sparse16 | 18.28 | 10.48 | 10.98 | 12, 12 |

The 256K-slot table improves K81,920 from 13.68 to 12.65 ms (about 8%), and K90,112 from 13.92 to 13.46 ms (about 3%). At K98,304 and larger, frozen v7 is faster. The larger 512K table usually loses to the smaller table here.

An **unexpected hardware-specific opportunity** appears on sparse16: ARM frozen v7 takes 18.28 ms versus 10.48 ms for 256K-slot counting. The same mechanism regresses x86: frozen v7 5.90 ms versus 10.59 ms. This suggests a separate ARM-only dispatch experiment, not a general enlargement.

## Verdict and remaining issues

**NO-GO** for adding 131K reconstruction unconditionally. The gains are strongly CPU- and cardinality-dependent, and all failed attempts include substantial overhead. A conditional method would need additional reliable dispatch and new held-out runs; sample-gate-only improvements were already rejected in v8.

Potential next effort: On ARM (non-BMI2), route very-low-variable-bit noncontiguous 64-bit values to an exact bounded frequency engine, retaining x86 compact-bit counting. Verify direct runtime and exactness against frozen v7, VQSort and IPS4o.

No SOTA or novelty claim, no merge. Source and user proprietary LICENSE unchanged.

## Raw evidence
- [x86 seed measurements](x86_epyc7763_raw.csv)
- [ARM seed measurements](arm64_neoverse_n2_raw.csv)
- [x86 summary](x86_epyc7763_summary.csv)
- [ARM summary](arm64_neoverse_n2_summary.csv)
