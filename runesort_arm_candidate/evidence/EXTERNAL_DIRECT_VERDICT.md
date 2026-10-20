# Native ARM64 sparse-bit experiment: full direct competitor results

**Research status:** GO for targeted ARM64 candidate review, NO-GO for generic/SOTA claims. Draft PR #5, unmerged; existing main and proprietary LICENSE untouched.

## Reproducible evidence

- [Completed two-architecture full baseline comparison](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39385568146) — **both ARM64 and x86 jobs passed**, every output verified against `std::sort`.
- [Independent initial mechanism ablation](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39385296411) and [successful full CMake test suites](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39385568112).
- ARM native Neoverse-N2 and x86 AMD EPYC 7763; GCC 13.3 C++20 `-O3 -march=native -DNDEBUG`.
- Exactly the same ascending `uint64_t` arrays and oracle per case. N=100K and 1M, 10 workload families, 3 seeds (271828/314159/777777), 4 measured repetitions after 1 warmup, shuffled algorithm order. Times are median of seed medians; input generation/cloning/verification outside timed region, sort allocations included.
- Five algorithms: ARM candidate, frozen v7, pinned Highway VQSort, pinned sequential IPS4o, Boost pdqsort. No CAFS ARM comparison because the published CAFS implementation uses x86-specific SIMD instructions. Do not pool x86-64 and ARM raw durations or claim cross-CPU speedups.

## ARM64 Neoverse-N2 — N=1,000,000 milliseconds

| Distribution | Candidate | Frozen v7 | VQSort | IPS4o | Winner |
|---|---:|---:|---:|---:|---|
| sparse16 | 13.458 | 19.194 | 17.964 | 15.909 | candidate |
| uniform64 | 16.143 | 16.281 | 17.462 | 19.427 | candidate |
| random32 | 12.470 | 12.671 | 17.543 | 19.558 | candidate |
| K1024 | 2.608 | 2.623 | 8.316 | 8.034 | candidate |
| K65536 | 11.351 | 11.135 | 18.295 | 15.867 | frozen_v7 |
| nearly | 5.716 | 5.738 | 11.629 | 13.438 | candidate |

On sparse16, candidate 13.458 ms versus frozen v7 19.194 ms, VQSort 17.964 ms, IPS4o 15.909 ms and pdqsort 15.968 ms. **29.9% less runtime** than frozen v7 on this runner, and faster than every external comparator actually tested on sparse16. This is a workload-specific result, not overall SOTA.

At N=100K the ARM selector is disabled by its N>=524288 guard; the two RUNE headers should be functionally/algorithmically equivalent, with measured differences attributed to variation.

Winner counts across all N=1M ARM workloads: `{"candidate":5,"frozen_v7":5}`. These are not meaningful universal rankings; trivial differences between candidate and v7 may be noise.

## x86-64 AMD EPYC 7763 — N=1,000,000 milliseconds

| Distribution | Candidate | Frozen v7 | VQSort | IPS4o | Winner |
|---|---:|---:|---:|---:|---|
| sparse16 | 5.998 | 5.847 | 13.586 | 21.365 | frozen_v7 |
| uniform64 | 26.240 | 26.612 | 12.835 | 28.079 | VQSort |
| random32 | 17.913 | 17.676 | 12.940 | 28.238 | VQSort |
| K1024 | 3.974 | 3.960 | 5.427 | 10.082 | frozen_v7 |
| K65536 | 11.743 | 11.577 | 13.581 | 21.428 | frozen_v7 |
| nearly | 6.040 | 6.289 | 13.126 | 18.070 | candidate |

The ARM-only selector is excluded by compile-time `__aarch64__` guard. Any x86 candidate-v7 differences are uncaused timing variation, not a claimed gain or regression in algorithm logic. VQSort retains its high-entropy x86 superiority.

## Mechanism and exactness

- The full-array XOR-mask scout proves exactly 16 varying bit positions across all N elements. This is a real certificate, not a probabilistic first-64 sample.
- On ARM64 only and N>=524288, large numeric range, the candidate first tries v7's already present exact 81,920-key frequency reconstruction. A 16-bit variable mask proves at most 65,536 distinct possible values, less than the engine capacity.
- Exact key-equality comparisons resolve collisions; a bounded 32-probe rejection leaves the input unmodified and then executes frozen portable compaction. No unsafe assumption about unseen values is necessary.
- Adversarial tests include a random full-width outlier after 64 elements, plus frozen v7 differential/adversarial suite. Original RUNE value-sort semantics are unchanged.

## Status and next engineering work

**Support:** candidate's targeted ARM sparse16 speedup and correctness; a narrow architecture-gated change with no x86 pathway change.

**Still outstanding:** repeat on dedicated ARM machines (different microarchitectures) with isolated CPU/pinning, more held-out seeds, and measure memory alloc/cache profiles. Confirm benchmark results under more realistic workloads and payload-bearing keys where applicable. Only then decide merging into v7. The candidate remains a draft PR and proprietary LICENSE is retained.

- [ARM raw CSV](arm64_neoverse_n2_direct_raw.csv) · [ARM aggregate](arm64_neoverse_n2_direct_summary.csv)
- [x86 raw CSV](x86_epyc7763_direct_raw.csv) · [x86 aggregate](x86_epyc7763_direct_summary.csv)
