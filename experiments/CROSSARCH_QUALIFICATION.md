# RUNE v7: Independent ARM64 cross-architecture qualification

**Purpose:** Establish whether the RUNE v7 performance advantages on x86-64 AMD EPYC 7763 replicate on a native ARM64 CPU. This is a research experiment, not a release change. The sorter source, upstream proprietary LICENSE and prior benchmark files are unchanged.

## Design
- Matrix: GitHub native `ubuntu-24.04-arm` (ARM64) and `ubuntu-24.04` (x86-64).
- Exactly the same frozen v7/v6/v5 header versions from parent research branch on both runners.
- Comparators: pinned Google Highway **VQSort**, pinned sequential IPS4o, Boost pdqsort/spreadsort, and C++ std::sort. CAFS is intentionally excluded because this comparison needs all baselines to compile on both architectures; CAFS's AVX2/BMI-dependent code is covered by the prior x86 runs.
- 18 workload families: 16 preexisting plus K81920 (the exact fifth-tier capacity) and K90112 (above-capacity rejection), at N=100,000 and N=1,000,000.
- Three seeds, four measured rounds/seed, one warmup round, randomized sorter call order, same input and `std::sort` oracle for every algorithm. Generation, array copying, and validation are outside the timed region.
- CPU model, compiler, and pinned external commits recorded in Actions logs.
- Benchmark results on separate runners are independent and **must not be divided across architectures to form speedup ratios**.

## Acceptance
- All sorting outputs match `std::sort` on ARM64 and x86-64, including high-bit and sparse16 paths.
- N=1M wins at K40960, K49152, K65536, and K73728 are evaluated **against ARM-native VQSort and frozen v6 on the same runner**.
- Sparse16 must not regress severely against frozen v6, and K90112 must fail closed rather than corrupting data.
- Publish CPU-specific raw CSV and seed-median summaries, with negative results preserved.
- No global SOTA or new algorithmic novelty claim.

## Commands
See [matrix workflow](../.github/workflows/v7-crossarch.yml) and the source `experiments/v7-crossarch-competition.cpp`. GitHub Actions compiles Highway from the pinned source on each runner. Expect compiler flags / supported SIMD to differ across architectures by design.

**No merge without explicit review and authorization.**
