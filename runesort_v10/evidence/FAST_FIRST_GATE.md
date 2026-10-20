# RUNE v10 preliminary same-run evaluation — 2026-10-20

## Status
- **Build and tests:** [GitHub Actions run 39399619256](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39399619256) succeeded.
- **RUNE v10 versus frozen v7:** [CI run 39399619326](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39399619326) succeeded on ARM64 and x86; every output was identical to independent std::sort oracle.
- Five fixed previously unused seeds, 1 warmup + 4 measured repetitions per seed; median of five seed-level medians; same array per algorithm, randomized algorithm ordering; excludes input generation/working-copy cost, includes sorting allocation.
- AMD-versus-Intel / CPU-versus-ARM timings **must not be divided against one another**.
- Hardware: ARM64 **Neoverse-N2**, x86-64 **Intel Xeon 6973P-C**; GCC 13.3 on Ubuntu 24.04 `-O3 -march=native -DNDEBUG -std=c++17`.
- Workloads: N=1,000,000, 64-bit keys; `K>0` means equally selected from K random full-width key candidates, `K=0` uniform random64, `K=-1` random32, `K=-2` narrow 16-bit range. Small seed-level differences and VM noise cannot show statistical significance.

## Targeted one-million-element results (milliseconds)

| K | ARM v10 | ARM v7 | x86 v10 | x86 v7 |
|---:|---:|---:|---:|---:|
| 81920 | 14.043 | 14.627 | 12.470 | 13.331 |
| 90112 | 13.910 | 14.362 | 12.495 | 13.373 |
| 98304 | 13.787 | 14.200 | 12.329 | 13.428 |
| 114688 | 13.799 | 14.115 | 12.450 | 13.373 |
| 131072 | 13.700 | 13.871 | 12.918 | 13.579 |
| 147456 | 13.731 | 13.860 | 12.528 | 13.482 |
| random64 | 14.937 | 14.941 | 13.155 | 13.140 |

Observed improvement for K~82K–131K on both machines is approximately **1–9%**. Mixed or zero improvements outside that band; no major algorithmic novelty claim. The result is *initial empirical support*, not enough for merging.

The expanded [external-sorter qualification](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39399873667) uses five **additional held-out seeds** and compares against pinned Highway VQSort, IPS4o and Boost sorters, not just RUNE v7.

## Raw data
- [ARM raw](arm64_neoverse_n2_fast_raw.csv), [ARM summary](arm64_neoverse_n2_fast_summary.csv)
- [x86 raw](x86_intel_6973pc_fast_raw.csv), [x86 summary](x86_intel_6973pc_fast_summary.csv)

**Decision:** Keep draft. No merge. Large future comparisons require stable dedicated CPU clocks and cache counters.
