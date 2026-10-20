# RUNE v8 two-stage sampling experiment: negative benchmark result

**Verdict: NO-GO for promotion.** The pilot code is correct within tested bounds, but the intended performance advantage over RUNE v7 was not demonstrated. Preserve this research PR without merging into v7 or `main`.

## Provenance
- CI [run #39378984371](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39378984371): **both native ARM64 and x86-64 benchmark jobs completed successfully** and all outputs matched std::sort.
- Compiled code revision `9791f836a03dff25b8094642c652559add1cffbe`; subsequent additions changed **documentation and reproducibility material only**, not the sorter or benchmark.
- ARM64: Neoverse-N2, Ubuntu 24.04; x86-64: AMD EPYC 9V74, Ubuntu 24.04; GCC 13.3, native C++20 `-O3 -march=native -DNDEBUG`. This specific x86 CPU differs from the EPYC 7763 in the parent v7 report; do **not** pool performance numbers across CPUs.
- Nine implementations: RUNE v8, frozen v7/v6/v5, Highway VQSort, sequential IPS4o, Boost pdqsort and spreadsort, std::sort. VQSort/IPS4o from pinned upstream sources in workflow.
- At N=100K and 1M: 18 workload families including boundary K73728–K90112. Five **new** independent seeds, one warmup, four measured repetitions/seed with shuffled sorter execution. Every result verified against std::sort.
- Listed times are median of five seed medians in milliseconds; raw CSV includes seed-level min/max. Input generation, cloning, and verification are excluded from timed regions; sorting-internal memory allocation included.

## ARM64 Neoverse-N2, N=1M

| Case | v8 ms | v7 ms | v6 ms | VQSort ms | Winner |
|---|---:|---:|---:|---:|---|
| K40960 | 9.053 | 9.109 | 13.666 | 17.368 | RUNE_v8 |
| K65536 | 11.053 | 11.121 | 13.128 | 18.094 | RUNE_v8 |
| K73728 | 12.075 | 11.781 | 13.019 | 18.055 | RUNE_v7 |
| K74752 | 12.052 | 11.811 | 12.919 | 18.017 | RUNE_v7 |
| K76800 | 12.339 | 12.118 | 12.897 | 18.004 | RUNE_v7 |
| K77824 | 12.990 | 12.965 | 12.847 | 17.994 | RUNE_v5 |
| K79872 | 12.969 | 12.962 | 12.862 | 18.017 | RUNE_v6 |
| K81920 | 12.890 | 12.855 | 12.832 | 18.005 | RUNE_v5 |
| K90112 | 12.743 | 12.841 | 12.654 | 18.080 | RUNE_v6 |
| sparse16 | 18.457 | 18.427 | 18.797 | 17.759 | IPS4o_seq |
| uniform64 | 13.344 | 13.390 | 13.708 | 17.338 | RUNE_v5 |

Winner counts across all 18 workloads: `{"RUNE_v5":4,"IPS4o_seq":1,"RUNE_v6":5,"RUNE_v8":5,"RUNE_v7":3}`. At 100K: `{"RUNE_v5":7,"RUNE_v7":3,"VQSort":1,"RUNE_v8":5,"RUNE_v6":2}`.

## x86-64 AMD EPYC 9V74, N=1M

| Case | v8 ms | v7 ms | v6 ms | VQSort ms | Winner |
|---|---:|---:|---:|---:|---|
| K40960 | 7.015 | 7.053 | 12.042 | 6.048 | VQSort |
| K65536 | 9.730 | 9.668 | 14.122 | 5.989 | VQSort |
| K73728 | 10.926 | 10.610 | 14.506 | 5.968 | VQSort |
| K74752 | 10.970 | 10.702 | 14.616 | 5.977 | VQSort |
| K76800 | 11.298 | 10.900 | 14.789 | 5.973 | VQSort |
| K77824 | 14.813 | 14.728 | 14.787 | 5.972 | VQSort |
| K79872 | 14.955 | 14.871 | 14.992 | 5.968 | VQSort |
| K81920 | 15.055 | 15.213 | 14.965 | 5.974 | VQSort |
| K90112 | 15.373 | 15.221 | 15.269 | 5.949 | VQSort |
| sparse16 | 4.946 | 4.669 | 4.663 | 5.976 | RUNE_v6 |
| uniform64 | 9.944 | 9.858 | 10.034 | 5.789 | VQSort |

Winner counts across all 18 workloads: `{"VQSort":15,"RUNE_v6":2,"RUNE_v7":1}`. At 100K: `{"VQSort":15,"RUNE_v8":1,"RUNE_v6":1,"RUNE_v7":1}`.

## Refutation analysis

The *pilot-only* success criterion was met in both a 20-seed training set and a disjoint 20-seed held-out set: all tested K73728 cases were admitted by v8 while K81920 was rejected. This calibration did **not** translate into a useful net complete-sort speedup:

- At K73728, ARM64 v8 is slower than v7 (12.075 versus 11.781ms), and on x86 v8 10.926ms versus v7 10.610ms. The second 32K sample creates additional overhead when v7's single-sample decision would have been profitable.
- At K76800, ARM64 v8 12.340ms versus v7 12.118ms; on x86 v8 11.298ms versus v7 10.900ms.
- At K81920, ARM64 v8 12.890ms versus v7 12.855ms; on x86 v8 15.055ms versus v7 15.213ms (small improvement, not material compared with VQSort 5.974ms).
- On the new x86 EPYC 9V74, Highway VQSort is markedly faster on almost all tested cardinalities, including **K40960 (6.048ms vs RUNE v8 7.015ms)** and **K81920 (5.974ms vs v8 15.055ms)**. Results are CPU-dependent; the earlier EPYC 7763 v7 ranking is not contradicted by differing VM hardware.
- Small changes on K40960/K65536 are within an environment where overhead and scheduling noise can explain results. Do not present those differences as an improvement.

**Mechanistic conclusion:** Smarter gating can avoid a rare unnecessary fifth-tier scan, but the dominant K81920 cost comes from the slow general fallback (or an unprofitable reconstruction), not from the pilot alone. Improving the fallback or adding a truly efficient larger exact tier is the stronger research direction.

## Correctness and next hypothesis

- The v8 CMake [build-and-test run #39378772065](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39378772065) succeeded; updated documentation/benchmark [run #39378984347](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39378984347) succeeded.
- The bounded 64-probe sample path cannot alter any array values; misclassification only affects work selection. Exact fifth reconstruction retains v7's fail-closed guarantee.
- This experiment adds no proof of novelty and does not substantiate overall SOTA sorting. Avoid merging PR #4 or presenting it as v7's successor.
- Next experiment (separate candidate): exact frequency reconstruction with a larger 131K-key tier, or a more efficient generic fallback, evaluated against frozen v7, VQSort and CAFS. Demand actual net wins on two CPU architectures and careful memory-cost accounting.

## Evidence files
- [ARM64 raw measurements](arm64_neoverse_n2_raw.csv) and [aggregates](arm64_neoverse_n2_summary.csv).
- [x86 raw measurements](x86_epyc_9v74_raw.csv) and [aggregates](x86_epyc_9v74_summary.csv).
- [Pilot training duplicate counts](pilot_calibration.csv) and [held-out counts](pilot_heldout.csv).
- [Reproduction instructions](REPRODUCE_PILOT.md) for pilot calibration; benchmark workflow resides under `.github/workflows/v8-crossarch.yml`.

**Decision: Research hypothesis refuted by direct benchmark. Keep draft, do not merge.**
