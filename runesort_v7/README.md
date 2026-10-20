# RUNE Sort v7 — exact 81,920-key frequency tier (experimental)

**Status:** stacked draft candidate on top of RUNE v6 PR #1; not merged. Original v5, v6 code, and proprietary LICENSE are unchanged.

## Algorithm

- Retains the exact 16K and 32K tiers from v6.
- Adds an exact bounded fifth tier with 262,144 hash slots, capacity **81,920 distinct uint64 keys**, 32-probe bound, exact key comparison, 32-bit counter-overflow rejection, and reconstruct-by-sorted-frequency-pairs.
- The original array remains unchanged on any failed reconstruction. An incorrect sampling estimate can affect only runtime, not sorting correctness.
- After the 4K-tier rejection, for n >= 524288 and >=100 duplicate observations, sample 16,384 evenly spaced values. Count exact distinct sample keys in a separate 32,768-slot table. Attempt fifth reconstruction only with  >=1,600 repeated sample values.
- Sample-count threshold is empirical and **not a proof of cardinality**. It is allowed to miss opportunities and have false positives; rejection remains correct.

## Preliminary local benchmarking

GCC 14.2, C++17, -O3 -march=native -DNDEBUG; one AMD EPYC 9V74 virtual CPU, n=1M, 4 independent seeds (271828,314159,777777,8675309), 5 measurements per seed, input copying outside timing. Times below are medians of seed medians, rounded. The results were observed in an interleaved v6/v7 comparison.

| Distinct keys | v6 ms | v7 ms | Relative |
|---|---:|---:|---:|
| 36864 | 13.39 | 7.51 | 1.78x |
| 40960 | 13.97 | 7.32 | 1.91x |
| 49152 | 14.64 | 8.62 | 1.70x |
| 65536 | 15.06 | 10.33 | 1.46x |
| 73728 | 15.53 | 15.60 | ~parity |
| Uniform random 64-bit | 11.14 | 11.16 | ~parity |

**Negative experiment:** Without the 16K-sample gate, failed 65K reconstruction caused ~9% regressions on K73728. The pilot minimizes those wasted attempts on these inputs, but adds its own overhead. Neither this heuristic nor its cutoff is universally optimal.

## Extended held-out boundary experiment

Eight **held-out** seeds (95101–95108), with three measured repetitions per seed, uncovered a K67,584 regression in the original 65K-capacity design. Widening the same 262,144-slot fifth table to 81,920 exact keys and reducing the pilot threshold to 1,600 repeated observations removed that regression in the tested 67K–78K band. Across those held-out seeds on the local EPYC 9V74 VM, K67,584 improved from about 14.94ms (v6) to 10.38ms (v7), and K73,728 from about 15.53ms to 11.11ms. This remains a non-SOTA, hardware-specific result; performance at the new boundary K81,920 was approximately at parity, and a 0–8% regression on some seeds remains possible. See `evidence/heldout_81k_boundary.csv` and `evidence/heldout_73k_band.csv` for input-level data.

## Sparse-bit dispatch regression and fix

The first external nine-way run (AMD EPYC 7763) exposed a large slowdown on a sparse 16-bit-varying-key workload: v7 10.93ms, frozen v6 5.70ms. The fifth tier was stealing work from v6's existing 16-bit bit-compaction counting path. v7 now requires the 64-element variability sample to contain **more than 16 varying bit positions** before attempting the fifth tier. This guard is performance-only and cannot affect correctness: a false negative simply uses existing exact fallback logic. A local four-seed sparse-bit retest shows near parity, and an adversarial differential case was added; independent CI rerun is required before promoting the result. The unfavorable original measurement remains part of the evidence rather than being erased.

## Verification

Locally executed targeted suite: 328 boundary and adversarial differential cases, under GCC portable -mno-bmi2 and with ASan+UBSan. Exercised N=0,1,2,63,64,65,4096,65536,262143,262144,524287,524288,525000; K=1,2,16,4096,16384,32767,32768,32769,40960,65535,65536,65537,73728,98304; hostile bucket collisions; random high-bit outliers; sorting exactness and failed-call input preservation.

## External competitors

This local result does **not** establish superiority over CAFS or Highway VQSort. Pinned direct-comparison workflow added for v7/v6/v5/CAFS/VQSort/IPS4o/pdqsort/spreadsort/std::sort. Record the exact commit SHA, runner CPU, and limitations for every result. Keep this stacked PR in draft pending evaluation. No novelty or overall SOTA assertion.

**Final revised pilot and bound:** 1,600 sample repeats; up to 81,920 distinct keys in 262,144 slots. The original 65K/1,820-gate run is retained in `evidence/local_four_seeds.csv` strictly as a historical pre-refinement measurement; it must not be cited as an evaluation of this new bound.
