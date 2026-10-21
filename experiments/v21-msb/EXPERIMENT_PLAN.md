# RUNE v21 — MSD partition + local exact sorting experiment

Pre-registered research question: can grouping a large unsigned uint64 array by the high byte once and processing smaller, cache-local buckets defeat the x86 global eight-pass scatter cost documented by v20? **No modification to production RUNE sorter, main, license, or previous negative findings**. Based on frozen v17 code snapshot; v18 buffering and v19 small histogram adjustments are not revived.

## Mechanisms

Candidate A, \`bucket_radix7\`: allocate O(n) scratch; count MSB=bits[63:56] (256 ordered buckets), then stable scatter every value to its high-byte bucket once. Sort each bucket's lower 56 bits with seven stable LSD byte passes, preserving the high-byte global order. For segments under 128 entries, run exact std::sort and copy. Reuse the same global scratch; start src in scratch and finish seventh pass into output array. Every bucket's histogram is bounded to its own segment. Complexity O(8n + 256*7*256) for n large, O(n) scratch. Stability of full record sort not promised: only uint64 key multiset/order.

Candidate B, \`bucket_stdsort\`: the identical high-byte scatter, then comparator-sort each bucket individually. O(n + Σ m_b log m_b) comparisons, O(n) scratch. This is intentionally structurally different from the same seven passes.

Correctness invariant: all keys with high byte b lie only in interval [bounds[b],bounds[b+1]); every value in interval sorted by remaining 56 bits (or its complete 64 bits, where the MSB is constant), so concatenating bins yields an exact globally sorted array. Array elements are never lost: MSB scatter fills each scratch destination exactly once; per-bucket pass counts and offsets form a permutation.

## Baselines and test controls

- Frozen RUNE17 full dispatcher and raw fixed8 worker.
- Pinned historical Google Highway VQSort (c3407ccaaecffa735c911cae3971efa02263211d) and IPS4o sequential (8d5a97bb4c4048a1eef18b537f191a31462b051a), plus standard sort.
- Two GitHub native ISA runners, 1M and 4M items, five fixed seeds (913103, 913109, 913117, 913123, 913129), five workload classes, one warmup + three measured repetitions, randomized method order. Copy input and build independent std::sort oracle outside timed region. All outputs must be exactly correct.
- Correctness suites: empty/singleton, 127/128 partition threshold boundaries, one-MSB-domain (e.g. random32), full64, duplicate-heavy, ordered, opposite extremes, 1M/2M random and one-MSB input; native CTest, both ISA ASan/UBSan, x86 no BMI2.

Primary x86 4M uniform64 target (same-run v17 and pinned VQSort), secondary ARM preservation. Sorted/alternating modes and forced fixed8 are controls, *not* user-facing dispatch claims. The prior v20 pinned VQSort x86 result (~55ms vs RUNE17 ~105.6ms) motivates experiment but cannot be compared numerically to new virtual CI runner timing: only same-run ratios count.

## Prior art / limits

MSD bucket partitioning is established prior art, cf McIlroy, Bostic & McIlroy (1993), *Engineering Radix Sort*; NIST DADS American flag sort. The present hybrid has **no novelty claim**. This is not in-place American flag permutation; it explicitly uses O(n) scratch. v21 must report all negative and positive seed-specific evidence. Do not merge without deterministic significant, repeatable target improvement, a hostile-input correctness sweep and follow-on controlled external qualification. This is a research PR only.
