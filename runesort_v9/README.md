# RUNE Sort v9 — Experimental exact 131,072-key tier

**Status:** research candidate, not a validated improvement. Stacked on frozen v7; rejected v8 pilot is intentionally excluded. Proprietary LICENSE and main branch unchanged.

## Algorithm
- Sixth frequency tier: 524,288 slots, at most 131,072 exact uint64 keys, bounded 32-probe collisions, counter overflow guard, successful sorted frequency reconstruction.
- Read-only on rejection, including key-capacity overflow, hash-probe exhaustion, or counter overflow.
- Reuses one 16,384-key read-only pilot with an optional exact repeat count. At n>=524288, if the early 4K-tier rejection observed at least 55 repeats and the 64-key variability mask has >16 bits:
  - pilot repeats >=1600 -> same v7 fifth-tier attempt;
  - 1024 <= repeats <1600 -> sixth-tier attempt;
  - otherwise -> established exact fallback.
- Thresholds are *empirical dispatch heuristics*. False classifications may degrade performance, never correctness.
- Estimated peak sixth-tier scratch: 524288*(8+4+1) = 6.5MiB plus 131072 pairs of 12–16 bytes (~2MiB) plus allocation overhead. The algorithm is single-threaded, unstable and sorts uint64 values only.

## Local preflight (isolated tier; *not* RUNE v9 sorter speed)
A standalone C++17 exact-table prototype was compiled on the local virtual CPU at -O3 -march=native -DNDEBUG; three seeds, one warmup and four timed sort-only calls, 1M keys. The tier accepted datasets K73728, K81920, K90112, K114688 and K131072. It rejected K147456 and more without array mutation. Isolated times at K81920 were about 11.5–12.1ms, at K90112 about 11.9–13.4ms, and at K131072 about 16–17ms. These microbenchmarks must not be compared with RUNE v7 on another machine. [Local raw measurements](evidence/isolated_tier_local.csv).

## Acceptance criteria
1. Current-source GitHub CI compiles and passes all inherited tests and new capacity/collision/fallback cases.
2. Direct same-CPU comparison with frozen v7, Boost pdqsort and std::sort at K73728, K81920, K90112, K114688, K131072, K147456, random64 and sparse16.
3. Go to the full CAFS/VQSort and ARM64 comparison *only if* the v9 dispatch yields robust end-to-end gains; preserve negatives, not just accepted keys.
4. No SOTA or novelty claim, no automatic merge.
