# RUNE v16b — two-stage boundary pilot evidence (before v16c refinement)

## Source and exact runs
Frozen experimental v16b sorter source commit `6b20be26645af0789d6b3c9e804c249771710380`; later commits only added tests and actions.
- [Initial v16a pilot direct test](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39453715755) 2/2 GREEN. First-stage repeat threshold changed from 1600 to 1450 at N≥2M, but produced false-positive attempts at K85000 with ~4–5% regression on two CPUs.
- [Revised v16b 2-stage mechanism](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39454361418) 2/2 GREEN; n=4M, nine K palettes × four independent input seeds, randomized v14/v16 order and identical exact sorting oracle.
- [Pinned native six-algorithm external comparison](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39454314650) 2/2 GREEN. V16b, frozen v14, v13, original ARM sparse16, pinned Highway VQSort, pinned IPS4o, n=1M/4M, eleven workload families, five seeds, four repetitions after warmup. **1,320/1,320 exact output records**.
- [Targeted ASan/UBSan](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39454756719) **2/2 GREEN** x86/ARM64, plus x86 no-BMI2. Full repository CMake tests passed.

## Mechanism
Original v14 16,384-sample repeat cutoff >=1600 misses K81920 (expected ~1534) on large arrays. Revised v16b uses a conditional, read-only 65,536-sample follow-up for ambiguous 1450..1599 first-sample repeats when n>=2M; threshold 20140. Expected 65K-sample repeats under i.i.d. uniform K81920 ≈20425; under K85000 ≈19852. These are model expectations and do not guarantee classification for adversarial distributions. The exact 81920-slot capacity limit, hash probe bound and non-mutation-on-reject fallback remain untouched; prefilter changes only select which exact algorithm to try.

## n=4M pinned external medians (ms, lower is better)
| CPU | K | Frozen v14 | v16b | VQSort | IPS4o |
|---|---:|---:|---:|---:|---:|
| AMD EPYC 9V45 | 81920 | 37.565 | **25.528** | **16.964** | 41.847 |
| AMD EPYC 9V45 | 85000 | 38.177 | 38.251 | **17.116** | 42.121 |
| ARM Neoverse-N2 | 81920 | 72.511 | **38.915** | 60.893 | 53.902 |
| ARM Neoverse-N2 | 85000 | 72.313 | 72.427 | 61.455 | **54.191** |

The ARM K81920 result is fastest among these named peers, but an **architecture/workload-specific empirical result**, not global SOTA. The x86 K81920 result is improved but still slower than VQSort.

## Critical newly uncovered counterexample
At K81920 n=4M, the new v16b dispatch works in **only three of five external seeds**:
- seeds 512003, 512011, 512021: large gains.
- seeds **512009 and 512047**: v16b time essentially v14 on both x86 and ARM64.
These failures match across CPU and input-seed identity. The upstream `extended_distinct_reconstruct` rejection duplicate gate `rejection_duplicates>=100` is a plausible cause because this count is near its sampling expectation at K81920. The new 65K pilot is designed to accept K81920 reliably; however the cause of these two per-seed fallbacks still needs direct instrumented confirmation, not guesswork. Do not claim 5/5 consistency or cherry-pick only the 3 winning seeds.

## Files
Raw from both initial (v16a) and corrected (v16b) four-seed mechanism trials, corrected x86/ARM external 660-row-per-CPU data, and a 264-row paired-seed summary are preserved alongside this report.

**Verdict v16b:** targeted improvement, CONDITIONAL research GO for ARM64 K81920, HOLD MERGE. Retain v14 as production reference and preserve all performance counterexamples.
