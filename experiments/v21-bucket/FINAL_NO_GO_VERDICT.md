# RUNE v21 — Completed cross-ISA MSD-bucket local-sort experiment

**Final verdict: NO-GO. DO NOT MERGE. No SOTA or novel-algorithm claim.**

Qualified source `5507ae7033bf9957d83f8f7e83bce62351bf41cf`; stacked on RUNE v20 research commit `d172516a691d82914b432a8d22bbaef55d828f91`. Existing production sorter, history, main and proprietary license remain unchanged.

## Verification

- [Research CI](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39549531266): **4/4 jobs passed** — native x86 AMD EPYC 7763 + ARM Neoverse-N2 full CMake/CTest, own oracle tests, externally pinned competitor benchmark; ASan/UBSan both architectures plus x86 no-BMI2.
- [Standard push CI](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39549531138) and [standard PR CI](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39549573225): **both passed**. **6/6 overall**.
- **600/600 measured full-array sort outputs matched independent `std::sort`** (300 per CPU), plus 355 independent array scenarios per CPU in the native and sanitizer suite.
- Same-array paired measurements, 5 pinned seeds `20270101,20270103,20270109,20270111,20270117`, 5 workloads, two scales (N=1M and 4M), six algorithms, one warmup and three randomly ordered measured runs, `-std=c++20 -O3 -march=native -DNDEBUG -pthread`. GitHub-hosted virtual x86/ARM machines are not controlled bare-metal.
- Historic exact external upstream code pinned to [Highway `c3407ccaaecffa735c911cae3971efa02263211d`](https://github.com/google/highway/commit/c3407ccaaecffa735c911cae3971efa02263211d) and [IPS4o `8d5a97bb4c4048a1eef18b537f191a31462b051a`](https://github.com/SaschaWitt/ips4o/commit/8d5a97bb4c4048a1eef18b537f191a31462b051a). Historic comparator version != global latest SOTA.

## Headline N=4M time medians (milliseconds, lower is better)

| ISA | workload | RUNE17 public | Bucket8 | Bucket12 | VQSort | Bucket12 / VQ paired latency |
|---|---|---:|---:|---:|---:|---:|
| x86_64 | uniform64 | 106.541 | 202.094 | 154.087 | 54.686 | 2.804× (0/5) |
| x86_64 | random32 | 69.680 | 201.284 | 154.947 | 54.779 | 2.829× (0/5) |
| x86_64 | K85000 | 83.304 | 156.345 | 108.187 | 55.633 | 1.943× (0/5) |
| x86_64 | sorted | 3.892 | 38.561 | 32.928 | 48.928 | 0.676× (5/5) |
| x86_64 | alternating | 3.619 | 58.319 | 58.385 | 2.574 | 22.678× (0/5) |
| arm64 | uniform64 | 61.530 | 138.756 | 112.952 | 75.344 | 1.499× (0/5) |
| arm64 | random32 | 46.972 | 138.787 | 113.766 | 75.566 | 1.508× (0/5) |
| arm64 | K85000 | 68.549 | 106.314 | 81.467 | 61.345 | 1.328× (0/5) |
| arm64 | sorted | 3.579 | 47.847 | 41.778 | 1.405 | 29.508× (0/5) |
| arm64 | alternating | 2.267 | 52.143 | 52.517 | 3.128 | 16.663× (0/5) |

Columns are medians of seed-level trial medians; paired ratios are *separate* medians of seed-matched ratios.

### Pre-registered primary target: x86 4M uniform64

- Frozen RUNE v17: **106.54 ms**; pinned VQSort: **54.69 ms**.
- RUNE v21 Bucket8: **202.09 ms**, Bucket12: **154.09 ms**. Both lose to VQSort and RUNE v17 **5/5 matched seeds** at the exact targeted input. The improved Bucket12 is still ~2.8× VQSort latency. This falsifies the v21 promotion hypothesis.
- ARM v21 uniform64: frozen v17 **61.53 ms**, Bucket8 **138.76 ms**, Bucket12 **112.95 ms**, pinned VQSort **75.34 ms**; both candidates lose.
- Selected forced sorted x86 input makes bucket local sort faster than VQSort, but the existing public RUNE sorter recognizes sorted input and is far faster (3.89ms vs candidate 32.93ms Bucket12). These controls cannot rescue the primary performance claim.
- Bucket-local sorter is **mathematically exact**: a full min/max scan identifies the highest varying bit and fixes all bits above; contiguous buckets cover those high variable bits monotonically, and local std::sort orders the remaining low bits. A test run is not a formal mechanical proof. Worst-case bucket skew still allows O(n log n) comparison work, with O(n + 2^B) scratch space.

## What was learned

The whole-array one-time scatter does not compensate for the cost of many local comparison sorts, so fewer global random scatters alone are insufficient on these targets. This mechanism differs materially from failed v18 per-bucket buffering, but reaches the same no-merge conclusion. v19 histogram micro-optimization also remains unpromoted. The underlying x86 full-width ~2× VQSort gap still exists. The next mechanism requires fundamentally different local key ordering (e.g. radix inside compact buckets with measured locality), or a SIMD-assisted partition/sort; those are **untested hypotheses**, not promised gains.

## Archived machine-readable evidence

- [x86 raw CSV](x86_64_raw.csv), [ARM raw CSV](arm64_raw.csv) — 300 timed output-checked rows each
- [20 paired architecture-size-workload groups](paired_summary.csv)
- [provenance.json](provenance.json)
- No merge or production promotion; retain negative research branch for reproducibility.
