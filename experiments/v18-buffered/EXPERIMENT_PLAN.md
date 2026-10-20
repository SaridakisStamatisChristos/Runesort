# RUNE v18 — cache-staged byte radix experiment

Research-only isolated worker, based on frozen v17 SHA `927d29b778b75f67e8a908cffa914a5f6e88929f`. No production dispatcher or license changes. DO NOT MERGE automatically.

Hypothesis: staging eight or sixteen uint64 keys per output bucket in 16/32KiB blocks then copying contiguous chunks reduces disruptive stores over the baseline eight-pass scattered writes. Counterhypothesis: extra bookkeeping and memcpy dominate, especially on small arrays or high-skew data. Neither is accepted without measurement.

Both algorithms use exactly eight stable LSD digits, eight fused 256-count histograms and O(n) scratch. Each bucket's buffered values are flushed in arrival order, hence every pass is stable and the final array ascending by the complete 64-bit key.

Verification on x86/ARM: CMake, std::sort full-array equality for small/boundary/extreme and 1M/2M random inputs, ASan/UBSan, x86 no-BMI2, 5 seeds, 3 measured rounds after warmup, randomized order with frozen v17 raw fixed8 worker, N=1M/4M on uniform64, random32, K85000, sorted and alternating. Raw timings archived.

This is a conventional buffered radix idea (prior art), not new asymptotic complexity or proven SOTA. No performance promotion without convincing target gains, controls, and subsequent external pinned VQSort/IPS4o comparison. v15 six-pass remains NO-GO.
