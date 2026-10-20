# RUNE v16c: final independent benchmark evidence

**Decision: CONDITIONAL GO for targeted cardinality research, NO-GO for general SOTA claim; HOLD MERGE.**

Source: frozen candidate commit `a0a25575931578de8aa6a871457944307d465caf` (based on v14 PR #13). Its proposed change is *read-only cardinality dispatch*, not the sorter's exact bounded frequency reconstruction. Previous v16a/v16b negative trials remain preserved.

### Final 2026-10-20 qualification
- [External pinned VQSort and IPS4o plus frozen RUNE candidates](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39456091599): **2/2 SUCCESS**, 660 exact-sorted results per CPU, **1,320/1,320** overall. Eleven modes × 2 sizes × five held-out seeds × six algorithms × two CPUs. Each row is four-measurement median after warmup, randomized run order, comparator checked elementwise to std::sort. GCC 13.3 native -O3, GitHub virtual CPUs.
- [Native isolated pilot mechanism](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39456091590): 2/2 PASS.
- [ASan/UBSan on ARM64 and x86](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39456091458): 2/2 PASS.
- [CMake build](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39456098909): PASS.

### Final external results, n=4,000,000, ms (lower is better)

| CPU | Workload | v14 | v16c | VQSort | IPS4o |
|---|---|---:|---:|---:|---:|
| AMD EPYC 7763 x86 | K81,920 | 94.485 | **37.121** | 55.144 | 67.213 |
| ARM Neoverse-N2 | K81,920 | 72.807 | **39.092** | 61.766 | 54.105 |
| AMD EPYC 7763 x86 | K85,000 | 95.57 | 96.606 | **55.94** | 67.522 |
| ARM Neoverse-N2 | K85,000 | 72.509 | 73.363 | 62.23 | **54.449** |
| AMD EPYC 7763 x86 | Uniform64 | 123.576 | 123.494 | **55.347** | 109.191 |
| ARM Neoverse-N2 | Uniform64 | 73.31 | 75.187 | 75.96 | 81.678 |

**K81,920 improved on 5/5 seeds on both architectures.** The final x86 runner was **AMD EPYC 7763**, not the **AMD EPYC 9V45** used for the earlier v16b external benchmark; its reversal of the v16b/VQSort ranking is a hardware/run distinction, not evidence that v16c universally beats VQSort on x86. At K85,000, v16c is ~1% slower than v14 and remains slower than external sorters; K90K, K131K and random full-width inputs show no general advantage. This negative evidence is intentionally retained.

### Mechanism and boundaries
V16a relaxed 16K-sample duplicate threshold but produced a K85K false positive. V16b added a separate larger 65K-sample pilot; two of five held-out K81,920 seeds still skipped due to an earlier 4,096-key rejection repeat gate. Instrumentation measured early repeats 108,89,105,100,97. V16c lowers that **early** condition from 100 to 80 only for N>=2M and retains *both* pilot conditions plus unchanged exact-capacity/hash collision validation; false pilot predictions can add cost but do not change correctness. The v16b report `V16B_VERDICT.md` remains available.

Raw: [x86](v16c_external_x86_raw.csv), [ARM64](v16c_external_arm64_raw.csv). Complete paired tables: [x86](v16c_external_x86_paired.csv), [ARM64](v16c_external_arm64_paired.csv). Differences below a few percent are not robust on shared virtualized hosts. Strong conditional ARM64/x86 K81,920 result is **not a universal SOTA proof**. No merge unless expressly approved.
