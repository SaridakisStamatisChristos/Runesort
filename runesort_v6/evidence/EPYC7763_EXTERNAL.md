# Independent RUNE v6 comparison — revised dispatch

**Reproduction status:** Completed and exact-output verified.
Source revision: [a5a89a28](https://github.com/SaridakisStamatisChristos/Runesort/commit/a5a89a28c49b11b0a01497c35c6a3032083c2083) (the later documentation/test additions leave the v6 sorter unchanged).

- [Eight-algorithm CI run](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39346413026), completed successfully
- GitHub-hosted **AMD EPYC 7763** virtual CPU, GCC 13.3, Ubuntu 24.04, `-O3 -march=native -DNDEBUG -std=c++20`
- Exact same `uint64_t` arrays across 8 implementations; oracle `std::sort` checked on every completed run
- Three independent seeds and four measured repetitions per seed; one warmup; medians of seed-level medians, not high-confidence universal means
- Sorting time excludes construction/copy/verification; includes sorting memory allocation
- Frozen external implementations: CAFS @ `13488afef446ae88e7ad631ec6f862def896094a` (pdqsort fallback), Highway VQSort @ `c3407ccaaecffa735c911cae3971efa02263211d`, IPS4o @ `8d5a97bb4c4048a1eef18b537f191a31462b051a`
- Distinct domains K are drawn as random full-width 64-bit keys, with equal-probability repeated sampling
- The prior cross-CPU AMD EPYC 9V45 run was for **pre-refinement v6**, and cannot be pooled with this exact-source benchmark.

## Principal results on AMD EPYC 7763, n=1,000,000

| Workload | v6 ms | v5 ms | CAFS ms | VQSort ms | Winner |
|---|---:|---:|---:|---:|---|
| K24576 | 6.938 | 20.471 | 17.261 | 13.553 | RUNE_v6 |
| K32768 | 7.959 | 22.027 | 17.859 | 13.260 | RUNE_v6 |
| K40960 | 23.004 | 23.135 | 18.122 | 13.403 | VQSort |
| uniform64 | 25.907 | 26.899 | 27.309 | 12.652 | VQSort |
| nearly | 4.925 | 4.582 | 9.061 | 12.903 | RUNE_v5 |

**Outcome:** RUNE v6 was fastest on **7 of 15** 1M workloads, VQSort won 4, and frozen v5 won 4 (mostly negligible differences). On 100K, RUNE v6 won 4 of 15 and VQSort won 8 — the benefit depends strongly on input size.

At n=1M, K24576: RUNE v6 6.938 ms versus v5 20.471 ms, CAFS 17.261 ms, and VQSort 13.553 ms.
At K32768: v6 7.959 ms, v5 22.027 ms, CAFS 17.859 ms, VQSort 13.260 ms.

## Caveats and negatives
- **Not a general SOTA result.** VQSort clearly wins uniform random and K40960/K49152.
- The 100K crossover means v6 is not always worth running; its fourth tier is gated at n>=262144, but the general path can still lose to competitors.
- A preliminary *different CPU and different dispatch revision* (EPYC 9V45) found VQSort substantially faster at K32768; reproducible cross-hardware assessment remains incomplete.
- CI runners were not dedicated; no CPU pinning, cache-miss measurements, robust confidence intervals, or hundreds of seeds.
- Mechanism is an engineering extension of known bounded frequency sorting. Novelty is not established.

## Raw data
- [All 720 seed/algo input-size measurements](epyc7763_raw_results.csv)
- [30 aggregate input-size/workload comparisons](epyc7763_summary.csv)
- [Reproduction source](../../experiments/v6-direct-competition.cpp)
