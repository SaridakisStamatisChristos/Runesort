# RUNE v9 exact 131K tier — CI benchmark verdict

**Status: MIXED / NO-GO for promotion**, 2026-10-20. This is a controlled research prototype stacked on frozen v7. No sorter code or proprietary license in parent branches was changed.

## Executed benchmark

- [GitHub Actions #39388306273](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39388306273): both native ARM64 and x86-64 nine-algorithm jobs succeeded, all sorted outputs matched reference `std::sort`.
- [Build/test #39388305969](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39388305969): passed all inherited and new sixth-tier correctness/collision tests.
- **Actual benchmark implementation commit:** `0bd4adbd9418c4e49604ed8ed7580c92e0313b8c`. Raw seed measurements and the report were added later, without changing benchmarked sorter source.
- ARM runner: Neoverse-N2, native `aarch64`; x86 runner: Intel Xeon Platinum 8573C virtual `x86_64`. GCC 13.3, C++20 `-O3 -march=native -DNDEBUG`. **Never compare raw runtimes across CPUs as speedup factors.**
- Five independent **held-out** seeds, 18 workloads, N=100000 and N=1000000, four measured repetitions/seed after one warmup. Sorting algorithms interleaved. Oracle, data generation and input copying outside timed window; internal sort allocations included.
- Nine real implementations: v9, frozen v7/v6/v5, pinned Google Highway VQSort, pinned sequential IPS4o, Boost pdqsort/spreadsort and C++ std::sort. CAFS excluded in the cross-architecture matrix because its x86-specific implementation is not ARM-compatible; direct x86 CAFS comparisons are available in earlier research.
- Exact goal: test whether an additional exact 131072-key table removes v7's K81920–K110592 performance cliff. All rejected table attempts are checked to leave input intact. The pilot is a performance heuristic, not a proof of cardinality.

## ARM64 Neoverse-N2, N=1M

| Workload | v9 ms | frozen v7 ms | VQSort ms | Winner |
|---|---:|---:|---:|---|
| K73728 | 11.786 | 11.873 | 18.020 | RUNE_v9 |
| K77824 | 12.147 | 12.879 | 18.028 | RUNE_v9 |
| K81920 | 12.855 | 12.922 | 17.979 | RUNE_v9 |
| K90112 | 13.101 | 12.898 | 17.993 | RUNE_v5 |
| K98304 | 12.686 | 12.737 | 18.009 | RUNE_v9 |
| K110592 | 12.540 | 12.595 | 18.025 | RUNE_v9 |
| K131072 | 12.334 | 12.300 | 17.888 | RUNE_v7 |
| local | 1.702 | 1.764 | 9.204 | RUNE_v6 |
| sparse16 | 18.689 | 18.679 | 17.760 | IPS4o_seq |
| uniform64 | 13.553 | 14.797 | 17.289 | RUNE_v9 |

Winner counts for all 18 workloads: `{"RUNE_v9":8,"RUNE_v5":4,"IPS4o_seq":1,"RUNE_v7":4,"RUNE_v6":1}`. At N=100K: `{"RUNE_v7":3,"RUNE_v9":4,"VQSort":1,"RUNE_v6":6,"RUNE_v5":4}`.

## x86 Intel Xeon 8573C, N=1M

| Workload | v9 ms | frozen v7 ms | VQSort ms | Winner |
|---|---:|---:|---:|---|
| K73728 | 12.367 | 12.483 | 6.453 | VQSort |
| K77824 | 14.446 | 18.017 | 6.715 | VQSort |
| K81920 | 18.429 | 19.090 | 6.050 | VQSort |
| K90112 | 15.644 | 19.301 | 6.766 | VQSort |
| K98304 | 19.555 | 19.288 | 6.767 | VQSort |
| K110592 | 19.408 | 19.412 | 6.329 | VQSort |
| K131072 | 19.339 | 18.593 | 6.009 | VQSort |
| local | 2.423 | 1.531 | 5.901 | RUNE_v5 |
| sparse16 | 6.273 | 6.234 | 6.056 | RUNE_v5 |
| uniform64 | 23.564 | 25.314 | 5.916 | VQSort |

Winner counts for all 18 workloads: `{"VQSort":13,"RUNE_v5":3,"RUNE_v7":2}`. At N=100K: `{"VQSort":15,"RUNE_v7":2,"RUNE_v5":1}`.

## Positive and negative findings

1. **Target improvement does not generalize:** x86 K90112 improved from v7 19.301ms to v9 15.644ms (1.23x) but Highway VQSort completed in 6.766ms. On ARM64 K90112 v9 was slower than v7 (13.101 versus 12.898ms).
2. **Capacity boundary remains:** At K81920 on x86, v9 18.429ms versus v7 19.090ms, but VQSort 6.050ms — a large gap. On ARM, v9 and v7 are effectively equal.
3. **Regressions:** x86 K98304 v9 19.555ms versus v7 19.288ms; x86 local-displacement v9 2.423ms versus v7 1.531ms (requires repetition to determine whether VM scheduling/cache effects or a real regression). ARM K90112 and K131072 show small losses.
4. **Specialization retained:** K73728 is near v7 on ARM and x86, and v9 preserves exact sorting on all tested workloads. No reason to claim global superiority.
5. **Hardware divergence:** ARM VQSort is slower than RUNE for many distributions, whereas on this x86 host VQSort won 13/18 at N=1M and 15/18 at N=100K. Earlier AMD EPYC 7763/9V74 rankings should not be pooled.
6. **Mechanistic limitation:** Enlarging the exact hash table alone leaves the cache-heavy frequency reconstruction and O(K log K) distinct-key sort. It can be worse than radix/scatter or SIMD-based sorting in some regimes. Tuning the pilot alone was already refuted in v8. A qualitatively different fallback or work-efficient representation deserves investigation.

## Auditability

- [ARM64 raw measurements](arm64_neoverse_n2_raw.csv) / [medians](arm64_neoverse_n2_medians.csv)
- [x86 raw measurements](x86_intel_xeon_8573c_raw.csv) / [medians](x86_intel_xeon_8573c_medians.csv)
- [Reproducible benchmark source](../../experiments/v9-crossarch-competition.cpp) and [workflow](../../.github/workflows/v9-crossarch.yml)
- [Candidate source](../rune_sort.hpp) and [differential tests](../test_v9_targeted.cpp)

**Decision:** HOLD draft; do not merge v9. Next discriminating improvement target should be cache-aware sparse counting or a genuinely faster general-purpose fallback, with a pre-registered held-out multi-CPU evaluation. No novel-first or overall state-of-the-art claim is supported.
