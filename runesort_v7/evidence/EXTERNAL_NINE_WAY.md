# RUNE v7 direct nine-algorithm competition — independent CI evidence

**Research status:** correctness-tested, workload-specific, **not an overall SOTA claim**. Source and experiments remain in stacked draft PR #2. Proprietary LICENSE and `main` unchanged.

## Final corrected-source benchmark

- Final exact-source [GitHub Actions run](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39370329797), **success**, commit `de0a1b3c630b7ee14f84ffb86f7f8656678fb5f7`.
- **AMD EPYC 7763 virtual** GitHub-hosted runner; GCC 13.3 Ubuntu 24.04, C++20 `-O3 -march=native -DNDEBUG`; one sorting thread. All implementations compiled against the same target CPU and sorted identical arrays on each seed.
- 9 implementations: RUNE v7, frozen v6, frozen v5, CAFS (pdqsort fallback), Google Highway VQSort, sequential IPS4o, Boost pdqsort and spreadsort, and `std::sort`.
- Three independent seeds (271828, 314159, 777777), one warmup and four measured repetitions per seed, interleaved randomized algorithm order. Numbers below are the **median of seed-level medians**; min/max are retained in the raw CSV. Generation and array-copy are excluded from timed intervals; sort-internal allocations are included.
- Every result checked against `std::sort`, benchmark job failed closed on mismatch. Two input sizes N=100000 and N=1000000, 16 workload families (including targeted cardinalities, random, sparse-bit, ordered and nearly ordered).
- External code revisions: CAFS `13488afef446ae88e7ad631ec6f862def896094a`, Highway `c3407ccaaecffa735c911cae3971efa02263211d`, IPS4o `8d5a97bb4c4048a1eef18b537f191a31462b051a`.

| Workload, N=1M | v7 (ms) | v6 (ms) | CAFS (ms) | VQSort (ms) | Fastest |
|---|---:|---:|---:|---:|---|
| K40960 | 8.444 | 23.221 | 20.579 | 13.628 | RUNE_v7 |
| K49152 | 9.340 | 23.390 | 20.789 | 13.626 | RUNE_v7 |
| K65536 | 11.321 | 24.387 | 27.428 | 13.589 | RUNE_v7 |
| K73728 | 12.343 | 24.382 | 27.598 | 13.564 | RUNE_v7 |
| sparse16 | 5.485 | 5.509 | 22.799 | 13.610 | RUNE_v7 |
| uniform64 | 28.172 | 26.894 | 27.432 | 12.948 | VQSort |
| nearly | 4.676 | 4.659 | 9.112 | 13.080 | RUNE_v6 |

At N=1M, fastest-workload counts are {"VQSort":2,"RUNE_v7":6,"RUNE_v5":4,"RUNE_v6":4}. At N=100K, {"VQSort":9,"RUNE_v6":2,"RUNE_v5":2,"RUNE_v7":3}. These counts are not a universal rank; many v7/v6/v5 differences on non-targeted cases are too small to distinguish from scheduler and cache noise.

**Supported finding:** v7's new bounded reconstruction tier wins the K40960, K49152, K65536, and K73728 equal-probability 64-bit-key distributions against CAFS and VQSort **on this particular AMD EPYC 7763 run**. VQSort is much faster on random 64-bit keys, and often on 100K arrays. RUNE v7 was 28.172 ms on random64 versus VQSort 12.948 ms.

## Preserved regression and corrective experiment

The [pre-guard run](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39369677789) succeeded functionally but caught a **performance regression** on the 16-varying-bit distribution: v7 10.926 ms, v6 5.705 ms. The fifth tier had intercepted data that v6's bit-compressed counting path handles faster.

The final commit adds a **performance-only** guard requiring more than 16 observed varying bits in the initial 64-element sample before attempting the fifth tier. On the same AMD EPYC 7763 family the corrected run showed v7 5.485 ms, v6 5.509 ms. Different GitHub Actions executions, not a perfectly controlled A/B for small differences, but the catastrophic slowdown disappeared.

Both full sets of 864 seed/algorithm measurements are archived separately:
- [Pre-guard raw data](pre_guard_epyc7763_raw.csv)
- [Final guard raw data](final_epyc7763_raw.csv)
- [Final aggregated dataset](final_epyc7763_summary.csv)

## Independent held-out tests, correctness and unresolved limits

- Eight local held-out seeds (95101–95108) identified a rejected-insert regression at K67584 with the initial 65536-key cap. Capacity was widened to 81920, same 262144 hash slots; K73728 success and the ~75K boundary tested locally. The pilot remains performance-only; rejected reconstruction does not mutate input.
- Locally passed 328 boundary/adversarial cases with AddressSanitizer and UndefinedBehaviorSanitizer, and portable compilation without BMI2. Current source also passed [GitHub CMake tests](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39370333881).
- Sampling may lead to unnecessary work or missed fast paths; failure is exact fallback. Tiny input and arbitrary signed/record sorting are outside the advertised RUNE specialization.
- The implementation is single-threaded, unsigned 64-bit value sorting, **not a stable key/value-record sort**.
- Performance comparison is on virtualized CPUs, 3 seeds only and without pinning, cache profiling, confidence intervals, or non-virtualized cross-architecture replication.
- **Novelty:** bounded open-addressing frequency sorting and sample-based dispatch have significant prior art. No new universally optimal sorting mechanism is proved. There is no basis for a global state-of-the-art or academic-first claim.

## Research decision

GO for **further targeted evaluation** of the K40K–74K and sparse-variable-bit dispatch on multiple dedicated CPUs, including an algorithm-specific breakdown of pilot overhead, cache misses, and the cardinality boundary.

NO-GO for a claim that RUNE v7 is globally the fastest sorting algorithm. Keep this PR as a **draft**, with v6 PR #1 and `main` unchanged until deliberate review.
