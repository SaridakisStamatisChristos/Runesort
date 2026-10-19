# RUNE v5 versus CAFS, Highway VQSort, IPS4o and C++ sorters
2026-10-20 — Direct external baseline evaluation (experimental branch only).

## Reproducibility
- **RUNE repository:** current experimental commit; core sorter is identical to the original v5 main-branch header.
- **CAFS** `kexibq-official/cafs-lib@13488afef446ae88e7ad631ec6f862def896094a`; `cafs2::cafs_sort` with Boost pdqsort fallback.
- **IPS4o** `SaschaWitt/ips4o@8d5a97bb4c4048a1eef18b537f191a31462b051a`; **sequential** `ips4o::sort`.
- **Highway VQSort** `google/highway@c3407ccaaecffa735c911cae3971efa02263211d`; dynamic dispatch `hwy::VQSort`.
- Boost pdqsort, Boost spreadsort, and standard C++ std::sort.
- GCC 13.3, `-O3 -march=native -DNDEBUG -std=c++20` on Ubuntu 24.04 GitHub hosted runners. Boost installed from Ubuntu apt.
- Each input array is the **same for all algorithms** on that case/seed; distinct random 64-bit keys for cardinality test, exact data and output semantics verified against std::sort before accepting timing. The working input copy, data generation, and verification are outside the measured interval; scratch allocation inside each sort is included.
- 3 independent seeds (271828,314159,777777), 4 measured repetitions per seed, 1 unmeasured warmup; algorithm call order shuffled per repetition. Reported ms is the **median of the 3 seed-level medians**. No CPU pinning and no statistical confidence claim.
- Two runs used **different CPUs** and must **never** be combined into a single machine-level ranking.

## Run 1: AMD EPYC 7763, all seven sorters
GitHub Actions [run 39342894032](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39342894032); job succeeded; all outputs verified. RUNE won 9/12 tested 1M workloads and VQSort won 3/12. At 100K RUNE won 7/12 and VQSort 5/12.

| workload | RUNE_v5 | CAFS_pdq | VQSort | IPS4o_seq | pdqsort | std_sort |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| uniform64 | 27.00 | 27.33 | 12.82 | 25.67 | 27.26 | 64.44 |
| sparse16 | 6.01 | 22.46 | 13.45 | 20.25 | 19.21 | 58.18 |
| K128 | 2.37 | 7.34 | 3.74 | 5.43 | 7.32 | 32.20 |
| K1024 | 3.96 | 8.29 | 5.44 | 10.17 | 9.85 | 40.65 |
| K4096 | 4.29 | 9.89 | 6.98 | 12.88 | 11.67 | 46.40 |
| K8192 | 4.44 | 12.97 | 7.97 | 13.91 | 12.85 | 48.83 |
| K16384 | 6.16 | 15.08 | 11.17 | 14.53 | 14.33 | 52.28 |
| K32768 | 22.24 | 16.39 | 13.44 | 16.67 | 16.55 | 55.59 |
| nearly | 4.62 | 9.10 | 13.06 | 17.57 | 9.06 | 11.12 |
| rotated | 2.17 | 12.28 | 12.03 | 16.28 | 12.05 | 8.44 |

**Findings:** RUNE leads from 128 to 16384 distinct keys in this test, and is especially strong on sparse-bit, local-displacement, nearly sorted, and rotated inputs. VQSort decisively wins on uniform64/random32 and K32768. The measured CAFS timings at some small K are surprisingly close to its pdqsort fallback: further fallback-path instrumentation is advisable before drawing mechanistic conclusions.

## Run 2: AMD EPYC 9V74, six sorters (no VQSort)
GitHub Actions [run 39343309296](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39343309296); job succeeded; all outputs verified. RUNE won 9/12 tested 1M workloads among the six included, and CAFS won 3/12. **This was not a seven-way competition** because VQSort was intentionally omitted.

| workload | RUNE_v5 | CAFS_pdq | IPS4o_seq | pdqsort | std_sort |
| ---: | ---: | ---: | ---: | ---: | ---: |
| uniform64 | 12.86 | 22.44 | 21.53 | 22.42 | 54.29 |
| sparse16 | 4.90 | 12.69 | 17.01 | 15.95 | 49.46 |
| K128 | 1.78 | 2.87 | 4.40 | 5.98 | 27.18 |
| K1024 | 3.15 | 2.99 | 8.19 | 8.01 | 34.05 |
| K4096 | 3.78 | 3.43 | 10.39 | 9.48 | 39.62 |
| K8192 | 3.46 | 3.96 | 11.26 | 10.46 | 41.71 |
| K16384 | 5.04 | 5.14 | 12.23 | 11.71 | 44.69 |
| K32768 | 11.61 | 6.42 | 14.34 | 13.57 | 47.55 |
| nearly | 4.42 | 7.51 | 14.77 | 7.46 | 9.47 |
| rotated | 1.93 | 9.95 | 14.13 | 9.74 | 6.93 |

**Findings:** CAFS wins K1024 and K4096, with a much larger lead at K32768. The difference at K16384 (RUNE 5.04ms vs CAFS 5.14ms) is too small to interpret robustly. RUNE wins on sparse-bit, rotation, and local-displacement patterns.

## Research conclusions
1. **RUNE is not universally fastest.** VQSort has a strong 64-bit high-entropy advantage; CAFS crosses over at different cardinalities depending on CPU.
2. **RUNE has genuine distribution-specific wins** against actual implementations, not only reconstructed benchmarks.
3. **Major current weakness:** dispatcher at K32768 and moderately sized inputs: reconstruction attempts add cost without yielding a final result.
4. **Next experiment:** measure whether CAFS is using its own fast path on both runners by counting fallback invocations, then implement a fail-closed branch policy for the 16K–32K transition.
5. **Do not claim global SOTA or algorithmic novelty** from two virtualized runners and 3 seeds. Conduct more seeds, dedicated hardware runs, vary N across 10K..10M, randomize array layout, track cache and memory effects, and test adversarial hash distributions.

## Source and result access
- [Seven-way implementation](../direct-competition.cpp)
- [Six-way implementation](../direct-cafs-ips4o.cpp)
- [Seven-way CI](../../.github/workflows/direct-sota.yml)
- [Six-way CI](../../.github/workflows/fast-cafs-ips4o.yml)
- [Aggregated raw median CSV](./summary_two_runners.csv)
- Original GitHub Actions logs and attached CSV artifacts are preserved on the two linked runs.
