# RUNE v19 — Compact histogram mechanism campaign

**Pre-registered hypothesis, no result yet.** Research parent is frozen v17, not v18 (NO-GO). No change to public dispatcher, main or proprietary license.

The original fixed8 worker uses 64-bit counters/offsets: 8*256*8 = 16 KiB. Test (1) fused uint32 histogram using 8 KiB and (2) per-digit uint32 histogram of 1 KiB, recomputed from current source on each of eight stable LSD passes. For N > UINT32_MAX both variants fall back to the independent v17 worker, ensuring valid offsets for all size_t values. Up to UINT32_MAX, no count, cumulative prefix sum or post-increment can exceed N, so uint32 is safe. Each worker does eight stable base-256 passes and therefore sorts all unsigned 64-bit keys exactly.

Hypothesis: smaller hot histograms improve x86 full-width uniform random throughput. Falsifiers: fused32 too costly from address calculations; perpass32 extra full-array scans outweigh footprint gain; regressions on ARM/random64 or other modes. Prior art: conventional fixed-width histograms; no novelty claim.

**Gate:** x86/ARM native builds, entire existing CTest suite, ASan/UBSan, portable x86 no-BMI2. Independent full-array std::sort comparisons, boundary/equal/extreme/large values. Paired same-input randomized order, 5 held-out seeds, one warmup, 3 measurement trials, N=1M and 4M, modes uniform64, random32, K85000, sorted, alternating. Store raw timing CSVs. Sorted/alternating forced fixed8 only; cannot be interpreted as public RUNE dispatcher latency.

**Promotion rule:** material repeatable x86 random64 improvement without ARM or correctness regressions, followed by production-dispatch test and pinned VQSort/IPS4o comparisons. Otherwise record a NO-GO; no merge. OmO controller uses native fallback when external bridge unavailable; independent evidence remains bounded to real CI machines and oracle semantics.
