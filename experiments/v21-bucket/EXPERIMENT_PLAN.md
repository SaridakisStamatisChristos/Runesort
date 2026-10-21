# RUNE v21 — One global MSD scatter and local bucket sorting

**State at launch: PROPOSED research, unmerged, not a production improvement.**

Parent: frozen v20 diagnostic commit `d172516a691d82914b432a8d22bbaef55d828f91`; full RUNE v17 source unchanged. Prior v18 buffered scatter and v19 32-bit histogram experiments remain NO-GO / unpromoted, respectively.

**Algorithm**: scan the minimum and maximum, choose the highest differing bit and the top B=8 or B=12 variable bits, count frequencies by that projection, perform *one* global scatter into contiguous ordered buckets, then `std::sort` each bucket locally and copy back. Order is exact since all bits above the chosen bucket window are constant across the entire input, and sorting within each bucket resolves the remaining low bits. Handle equal keys by no-op, empty/singleton as no-op, and extreme keys with exact `uint64_t` comparisons. Never misrepresent this conventional bucket/local-sort method as novel.

**Hypothesis**: one global scatter followed by cache-sized local sorts may outperform 8 random global scatters on x86 high-entropy full-width data. **Counterhypothesis**: output bucket allocation and comparator-based small sorts are more expensive than radix, particularly on skewed buckets and sorted controls. For worst-case all values in one bucket, worst-case comparator complexity remains O(n log n); extra memory O(n + 2^B).

**Qualification gate**: full-array oracle vs `std::sort` across 355 small/boundary/extreme cases plus 4 million-scale cases per ISA, ASan/UBSan both ISAs and x86 no-BMI2, existing CMake suite. Exact pinned external Highway VQSort and sequential IPS4o at same fixed commits used in v20, 5 held-out seeds, N=1M and 4M, 5 workloads (uniform64, random32, K85000, sorted, alternating), one warmup and 3 randomized measured trials per algorithm. Six algorithms × 5 seeds × 5 modes × 2 sizes × 2 hosts = **600 correct output checks required**. Compare complete v17 dispatcher, raw v17 fixed8, bucket8, bucket12, VQSort, IPS4o. Sorted/alternating candidate performance is a forced-worker negative control, not end-to-end RUNE.

**Primary target**: x86 4M uniform64, reproducible paired speedup vs **both** raw v17 and pinned VQSort, without egregious ARM/general regressions. No threshold tuning after results. If slower, archive NO-GO. If promising, HOLD merge until stronger A/B replication and latest pinned external comparison. GitHub virtual CPUs and historic external commits limit generality. No production release.
