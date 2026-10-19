# RUNE Sort v5 — Technical contract, proof, and evidence

## Model and semantics

Input: `n` valid consecutive unsigned 64-bit values. Output: same multiset, in nondecreasing unsigned order. In-place input mutation is permitted. Unstable with respect to equal-key records (only keys are accepted). Conventional single-threaded fixed-width RAM operations in C++17; benchmark metric is elapsed time for the sort call, not generation or input copying.

### New v5 mechanism

1. The inherited medium and extended tables can reconstruct outputs for up to 1024 and 4096 exact distinct keys respectively. Exact equality resolves collisions, so no approximation can contaminate output.
2. The extended table now optionally returns a **duplicate count on rejection**. Every time a key already present is encountered, the observation increments. This statistic is available even when the table ultimately rejects the input, because the entire attempt is read-only with respect to the original array.
3. If `n >= 262144`, the second-tier reconstruction rejects, and the observed count is at least 400, a 65,536-slot third table is attempted. It admits at most 16,384 distinct values and at most 32 probes per insertion/lookup.
4. On success, it sorts `(key,count)` pairs and writes each key `count` times. On probe, capacity, or counter saturation failure it returns without changing the input. All fallback paths remain exact.

**Why this dispatch helps:** A 4096-key table generally needs more processed observations before seeing its 4097th distinct key if the true domain is close to 8192 than if it is nearly unbounded. Number of repeats before failure is a very cheap cardinality indicator, not a correctness assumption. For one deterministic million-key test, observed duplicates were approximately 1607 at K8192, 634 at K16384, 260 at K32768 and 136 at K65536. Threshold 400 is *empirical*, may be CPU/data-distribution dependent and does not guarantee a successful third-tier attempt.

## Correctness argument (new tier)

- The table has three arrays `keys`, `counts`, `used` indexed by the same bucket. An unused slot has no key significance. An occupied slot's key is compared by exact 64-bit equality. A successful lookup of value `x` increments only the counter belonging to that value.
- For each input value x, the algorithm either adds a new (x,1) or increments its existing frequency exactly once. Open addressing and a finite probe bound may reject some legal inputs, but cannot cause incorrect acceptance because keys are matched exactly.
- Before every rejection, the algorithm only mutates its private arrays. Hence the original input is unchanged and a complete fallback is safe.
- On successful completion, the table represents the multiset exactly. Sorting the distinct keys ascending and emitting each key at its frequency reconstructs precisely the sorted input.
- The duplicate-dispatch criterion is never used as evidence that the input has bounded K. It merely decides whether to attempt an exact, fail-closed routine. Therefore a misleading sample or distribution cannot compromise correctness.

This is a human-checkable argument; it has not been formally verified in a proof assistant. Correctness of the full inherited sorter depends on the documented invariants of its other paths and is checked by differential testing, not globally formalized here.

## Bounds

For the third tier with S=65,536 slots, K<=16,384 distinct keys and at most 32 probes, successful scan plus output takes O(n + S + K log K) and O(S + K) auxiliary memory (under standard bounded hash operation assumptions); `std::sort` of the K pairs remains the main non-linear component. Since S and K have fixed implementation caps, this is O(n) for this particular fixed-width implementation. On rejection, the fallback retains the inherited full-sort worst-case O(n log n) time and O(n) auxiliary memory; time includes failed bounded scans.

A 32-bit frequency counter rejects rather than wraps on saturation. Extremely large n or unusual collisions may force fallback, preserving correctness but not performance. The additional storage of the third-tier table plus output pairs is roughly 1.1 MiB when the maximum K is reached, excluding allocator overhead.

## Verification run

- Six CMake suites passed (`correctness_compare`, `correctness_adversarial`, `correctness_v3_targeted`, `correctness_v4_targeted`, `correctness_v5_targeted`, `correctness_v5_dispatch`).
- The targeted v5 suite passed 930 checks and v5 dispatch tests explicitly exercised K=64, 4096, 8192, 16384, 32768, 65536, collision rejection, and late high-entropy outliers.
- Clang AddressSanitizer + UndefinedBehaviorSanitizer run passed for v5 targeted tests.
- GCC C++17 build without BMI2 and test run passed.
- Existing source-differential tests cover other paths, sizes, and distributions; no machine-checkable proof is claimed.

## Benchmarks and interpretation

- AMD EPYC 9V74 KVM virtual CPU; GCC 14.2; `-O3 -march=native -DNDEBUG -std=c++17`; one sorting thread; scheduler and frequency were not pinned.
- Exactly n=1,000,000 `uint64_t` values; independent seeds 271828, 314159, 777777, 8675309 (generator uses the seed plus mode index). 1 warmup + 5 measured repetitions per seed, interleaved five-algorithm order. Table values are medians of per-seed medians.
- Baselines: frozen RUNE v4, Boost pdqsort, Boost spreadsort and C++ std::sort, all sorted the same exact arrays, validated against std::sort output.

| Distribution | v4 (ms) | v5 (ms) | v4/v5 |
|:--|--:|--:|--:|
| K8192 | 12.5913 | 3.9362 | 3.199 |
| K16384 | 13.3580 | 5.8967 | 2.265 |
| K32768 | 15.1796 | 14.6645 | 1.035 |
| Uniform 64-bit | 12.9246 | 11.4601 | 1.128 |
| K4096 | 3.9127 | 4.1979 | 0.932 |
| K512 | 2.2604 | 2.2614 | 1.000 |
| K128 | 1.7489 | 1.7980 | 0.973 |
| Nearly sorted | 6.0615 | 6.3524 | 0.954 |
| Random 32-bit | 11.3168 | 11.8362 | 0.956 |
| Random 48-bit | 14.7851 | 15.1916 | 0.973 |

The strong K8192/K16384 wins are the chief validated improvement. Differences of 2–5% may be run-order, scheduler, or cache noise; do not claim high confidence from four seed medians alone. Pure random and nearly sorted performance remains mixed.

## Negative experiments

- **Progressively resized, single-scan frequency table:** despite eliminating an input scan, it lost badly on K128–K4096 because hash probes/cache behavior dominated. Candidate rejected.
- **Uninitialized radix scratch array:** tested and shown correct on covered radix paths, but failed to produce consistent speedup and sometimes regressed random 32/48-bit keys. Reverted in the release header.
- **Unconditional 65,536-slot tier:** improved K8192/K16384 but penalized high-entropy and K32768 workloads. The duplicate-observation gate made these penalties much smaller.

## Prior art and novelty

- CAFS (`https://github.com/kexibq-official/cafs-lib`) is directly relevant cache-aware low-cardinality frequency sorting. Its published claims are **not independently verified here**.
- Google Highway VQSort (`https://github.com/google/highway`) and IPS4o (`https://github.com/SaschaWitt/ips4o`) are important additional missing baselines.
- v5's added statistic-reusing tier and threshold are engineering innovations *within RUNE*, not necessarily an academic first. Formal novelty remains unchecked. Do not market RUNE as overall SOTA.

## Next discriminating experiment

Replicate this exact corpus and compiler settings on a pinned non-virtualized machine while adding directly compiled CAFS, VQSort, and IPS4o (with commits, ISA configuration, and license provenance). Compare across N from 10K to 10M, K from 256 to 64K, and adversarial repeat placement. Report distributions and confidence intervals, memory, and fallbacks. Reject the new tier if CAFS consistently dominates on equal workloads and no structural advantage emerges.