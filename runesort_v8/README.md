# RUNE Sort v8 — Two-stage cardinality pilot

**Status: experimental stacked change on top of RUNE v7, with all existing versions and the proprietary LICENSE unchanged.**

## Motivation
RUNE v7's exact 81,920-key frequency table can reject after a costly full scan near its capacity. On AMD EPYC 7763 with n=1M and K=81,920, v7 took 24.04ms, versus v6 23.87ms and VQSort 13.40ms. This problem is **performance only**: correctness and fail-closed reconstruction are unaffected.

## Modification
- Retain all v7 sorting mechanisms, hash tables, and 81,920-key exact capacity.
- Replace the uncalibrated 16,384-sample pilot by a two-stage *read-only* pilot.
- Exact repeated-key counts are taken with open addressing and full 64-bit key equality:
  1. Sample 16,384 evenly spaced keys, with 32,768 slots and bounded 64-probe rejection.
  2. If repeats >=1,760, accept; if repeats <=1,550, reject.
  3. Otherwise resample 32,768 keys with 65,536 slots. Attempt the fifth tier only if at least 6,000 repeats are observed.
- Sampling can make incorrect guesses about cardinality; **it never changes any input or assumes the sampled keys exhaust the input**. A false-positive leads to the same exact fifth-tier reconstruction, which validates every key and leaves the array unchanged on any failure. A false negative routes to the existing exact sorter.
- Pilot memory O(65,536) in its second stage; capped probe loop for collision-heavy data. Intended sorting semantics: ascending, unstable, unsigned 64-bit values only.

## Calibration — sample decisions only, not sort timings

A standalone exact pilot diagnostic was run locally on an **AMD EPYC 9V74 virtual CPU**. Each workload had N=1,000,000; 20 calibration seeds and 20 **different held-out seeds**. Both pilots sampled identical positions for each sample size, and all repeated-value counts used full-key equality. Counts below are accepted samples out of 20:

| Cardinality | v7 16K threshold calibration | v8 2-stage calibration | v7 held-out | v8 held-out |
|---|---:|---:|---:|---:|
| K=73,728 | 20 | 20 | 19 | 20 |
| K=74,752 | 19 | 20 | 20 | 20 |
| K=76,800 | 17 | 20 | 16 | 19 |
| K=77,824 | 12 | 16 | 13 | 13 |
| K=79,872 | 6 | 1 | 5 | 0 |
| K=81,920 | 1 | 0 | 0 | 0 |

This is a calibrated probabilistic dispatch on these particular uniform-domain distributions, **not** proof of error-free classification for arbitrary distributions. False decisions are performance effects, not correctness failures. In a local microbenchmark, 16K samples took ~0.055ms and 32K samples ~0.181ms on this CPU; those timings do **not** substitute for complete-sort benchmarks.

## Independent falsification benchmark

A pinned source comparison in [GitHub Actions](../.github/workflows/v8-crossarch.yml) benchmarks:
- RUNE v8, frozen v7/v6/v5, Google Highway VQSort, sequential IPS4o, Boost pdqsort/spreadsort and std::sort (nine total).
- Native ARM64 Neoverse and x86-64 Ubuntu runners.
- 5 **new, previously unused** seeds; 18 workloads, n=100K and 1M, 4 repetitions after one warmup.
- Every result checked against std::sort, interleaved algorithm order, array copying excluded from timing.

## Acceptance criteria

1. New v8 tests (including adversarial collisions and rejected input unchanged), inherited differential suites and portable C++ compilation pass on the CI runners.
2. At K81,920, v8 no longer loses materially to v7 or v6 because of a rejected full 81K table scan. At K90,112, v8 remains close to v7 or v6.
3. At K73,728 and K74,752, the new extra pilot must not destroy v7's win over VQSort and v6. Include negative findings.
4. No universality claim, no SOTA novelty claim, and **no merge without review**.

## Evidence files

- [20-seed calibration duplicates](evidence/pilot_calibration.csv)
- [20-seed held-out duplicates](evidence/pilot_heldout.csv)
- Direct benchmark files and summary will be added only after completed workflows.
