# RUNE v22 — Four-pass 16-bit radix: complete cross-ISA NO-GO result

**VERDICT: NO-GO FOR BOTH VARIANTS. DO NOT MERGE.** 16-bit LSD radix with four stable scatters is correct on tested inputs but much slower than the frozen v17 worker and historical pinned Highway VQSort on the x86 full-width target. The hypothesis is empirically refuted on the tested runners, not disproven for all machines.

## Actual verified runs

- Qualified implementation commit `80d0415c60844dbedd79f6a5702ba861ec1b74b8` on research branch based on v20 evidence commit `d172516a691d82914b432a8d22bbaef55d828f91`.
- [Research qualification CI](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39550955682): **4/4 jobs green** (native x86, ARM, x86 ASan+UBSan, ARM ASan+UBSan; x86 no-BMI2 portability).
- [Standard push CI](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39550955569) and [PR CI](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39550998337): green. **6/6 total**.
- **600/600 benchmark outputs compared element-by-element to independent std::sort**, including 300 each on x86 and ARM, all correct. Native and sanitizer differential tests cover empty/singleton/boundary/extreme/duplicate/narrow-domain arrays and large random arrays.
- GCC 13.3, Ubuntu 24.04 native x86 AMD **EPYC 9V74** (4 vCPU), ARM Neoverse-N2 (4 vCPU), `-std=c++20 -O3 -march=native -DNDEBUG -pthread`. **Important: EPYC 9V74 differs from earlier EPYC 7763 benchmarks**. Never divide timings from those independent runs. Comparisons are randomized, paired within each same-run set of exact identical arrays.
- Five pinned input seeds `310901,310907,310913,310919,310927`, N=1M and 4M, five distributions, one warmup and three recorded trials per algorithm; six algorithms, all timed sorting operations include candidate scratch/histogram allocation. Full raw timings retained.
- Historic external source revisions match v20/v21: Google Highway `c3407ccaaecffa735c911cae3971efa02263211d`, sequential IPS4o `8d5a97bb4c4048a1eef18b537f191a31462b051a`. They are pinned historical comparators, **not proven latest**.

## Four-million element evidence (milliseconds, lower is better)

| Architecture | Mode | RUNE17 dispatcher | Frozen fixed8 | Fused16 | Perpass16 | VQSort | Paired fused16/VQ latency |
|---|---|---:|---:|---:|---:|---:|---:|
| x86_64 | uniform64 | 41.74 | 40.72 | 77.09 | 77.70 | 25.46 | 3.018× |
| x86_64 | random32 | 34.45 | 35.13 | 44.82 | 47.09 | 25.49 | 1.756× |
| x86_64 | K85000 | 46.33 | 44.18 | 59.15 | 62.13 | 26.43 | 2.241× |
| x86_64 | sorted | 2.84 | 40.83 | 76.51 | 77.46 | 24.66 | 3.095× |
| x86_64 | alternating | 2.19 | 31.40 | 20.62 | 21.41 | 1.82 | 11.344× |
| arm64 | uniform64 | 63.16 | 62.59 | 96.81 | 85.84 | 75.34 | 1.285× |
| arm64 | random32 | 47.59 | 77.98 | 69.04 | 78.00 | 75.49 | 0.913× |
| arm64 | K85000 | 68.55 | 68.05 | 77.68 | 66.28 | 61.55 | 1.263× |
| arm64 | sorted | 3.56 | 62.72 | 90.65 | 85.07 | 1.38 | 65.250× |
| arm64 | alternating | 2.34 | 89.08 | 48.06 | 61.31 | 3.10 | 15.240× |

Columns show medians of five seed-level 3-trial medians. Ratio column is independently the median of matched seed-level ratios, not ratio of displayed medians.

## Analysis / pre-registered target falsification

- On the x86 4M `uniform64` target, **RUNE v17 41.74 ms, fixed8 40.72 ms, VQSort 25.46 ms** versus **fused16 77.09 ms and perpass16 77.70 ms**. Both variants lose to the pinned VQSort **5 of 5 seeds** and lose to frozen fixed8. Fused16 matched median time is `3.018×` of VQSort; **NO-GO**.
- On ARM `uniform64` 4M, frozen RUNE17 63.16 ms, VQSort 75.34 ms, fused16 96.81 ms, perpass16 85.84 ms. Both degrade compared with frozen RUNE.
- A narrower 1M x86 `random32` shows some improvement of either radix16 worker relative to RUNE17, but both still lose to VQSort, and both fail primary 4M uniform64. Such a narrower isolated win cannot overturn the prospective gate.
- Eight scatter passes cut to four, but 65,536-entry histograms create much larger random counter/offset working sets. That is a **plausible** cause of regression, **not proven by hardware counters**.
- Forced-worker sorted/alternating controls do not represent the behavior of existing RUNE's production dispatcher (which has specialized checks). No comparison-only exception may justify promoting the kernel.
- Stable LSD over four 16-bit digit positions covers the 64-bit order exactly; for N>UINT32_MAX the source safely falls back to frozen v17 due to 32-bit histogram counters. The work is conventional radix engineering with known prior art; no originality claim.

## Evidence and remaining uncertainty

- [x86 exact-output raw observations](x86_64_raw.csv): 300 rows.
- [ARM exact-output raw observations](arm64_raw.csv): 300 rows.
- [Cross-architecture same-run paired summary](paired_summary.csv): 20 CPU-size-input groups.
- [Provenance and decision record](provenance.json).

**Decision:** retain as a rejected, unmerged research branch. Do not re-run v15 six-pass 11-bit, v18 buffering, v21 MSD-local comparison sorting, or promote the v19 ~0.3% histogram tweak without genuinely new causal evidence. Further work should investigate x86 store/cache traffic with hardware PMU when possible, or a SIMD-assisted sorting/partition mechanism with pinned external comparisons.
