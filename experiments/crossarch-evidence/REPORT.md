# RUNE v7 native ARM64 and x86-64 benchmark qualification

**Evidence date:** 2026-10-20. **Verdict:** distribution-specific gains **replicated** on ARM64 and x86-64; global SOTA **not** demonstrated.

## Experimental source and completed CI run

- [Successful two-runner GitHub Actions comparison](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39377127482) — both ARM64 and x86-64 jobs completed and exact-output checked
- [Earlier ARM64-only successful replicate](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39377043896) — x86 metadata stage failed from a shell broken pipe; the old ARM job itself passed
- GitHub Ubuntu 24.04 ARM runner: **Neoverse-N2 aarch64**; x86 runner: **AMD EPYC 7763 x86_64**. GCC 13.3, native `-O3 -march=native -DNDEBUG -std=c++20`.
- RUNE v5/v6/v7 are frozen repository headers; no sorter modifications in this experiment.
- 8 algorithms per runner: v7, v6, v5, Google Highway VQSort, sequential IPS4o, Boost pdqsort, Boost spreadsort, and C++ std::sort.
- CAFS is not in this cross-architecture matrix; its implementation requires x86 SIMD/instructions. Prior x86-only CAFS comparisons remain in the parent research branch.
- External pin: Highway `c3407ccaaecffa735c911cae3971efa02263211d`; IPS4o `8d5a97bb4c4048a1eef18b537f191a31462b051a`.
- Each algorithm processes exactly the same array within a case/seed; output checked against std::sort before timing is accepted. 3 independent seeds, warmup plus 4 measured iterations per seed, randomized call order; inputs N=100K and N=1M in 18 distributions. Report values are median of per-seed medians, in **milliseconds**.
- All input generation, input cloning, and oracle validation are outside the sort timing; scratch allocation internal to sort is included.
- CPU-to-CPU raw time **must not be directly compared** as a speedup. Each benchmark ranking is within a single runner. No CPU affinity, dedicated host, confidence intervals, energy or perf counters.

## ARM64 Neoverse-N2 — 1,000,000 keys

| Distribution | RUNE v7 | RUNE v6 | VQSort | IPS4o | Winner |
|---|---:|---:|---:|---:|---|
| K40960 | 8.78 | 13.79 | 17.29 | 13.95 | RUNE_v7 |
| K49152 | 9.41 | 13.50 | 17.99 | 14.53 | RUNE_v7 |
| K65536 | 10.76 | 13.24 | 17.95 | 15.54 | RUNE_v7 |
| K73728 | 11.48 | 13.04 | 17.86 | 15.93 | RUNE_v7 |
| K81920 | 13.08 | 12.83 | 17.98 | 16.35 | RUNE_v6 |
| K90112 | 12.85 | 12.95 | 17.85 | 16.64 | RUNE_v5 |
| sparse16 | 18.60 | 18.61 | 17.71 | 15.62 | IPS4o_seq |
| uniform64 | 15.43 | 15.24 | 17.23 | 19.20 | RUNE_v6 |

Winner counts on all 18 one-million-key workloads: `{"RUNE_v6":4,"RUNE_v7":9,"IPS4o_seq":1,"RUNE_v5":4}`; at 100K: `{"RUNE_v7":6,"VQSort":1,"RUNE_v5":8,"RUNE_v6":3}`.

- [Raw ARM64 rows](arm64_neoverse_n2_raw.csv)
- [ARM64 median table](arm64_neoverse_n2_summary.csv)

## x86-64 AMD EPYC 7763 — 1,000,000 keys

| Distribution | RUNE v7 | RUNE v6 | VQSort | IPS4o | Winner |
|---|---:|---:|---:|---:|---|
| K40960 | 8.22 | 23.21 | 13.45 | 17.60 | RUNE_v7 |
| K49152 | 9.10 | 22.95 | 13.41 | 18.48 | RUNE_v7 |
| K65536 | 11.01 | 23.81 | 13.39 | 20.10 | RUNE_v7 |
| K73728 | 12.07 | 23.79 | 13.36 | 20.73 | RUNE_v7 |
| K81920 | 24.04 | 23.87 | 13.40 | 21.33 | VQSort |
| K90112 | 24.14 | 23.99 | 13.39 | 21.84 | VQSort |
| sparse16 | 5.84 | 5.70 | 13.40 | 20.22 | RUNE_v6 |
| uniform64 | 26.47 | 26.04 | 12.92 | 26.04 | VQSort |

Winner counts on all 18 one-million-key workloads: `{"VQSort":4,"RUNE_v6":4,"RUNE_v5":3,"RUNE_v7":7}`; at 100K: `{"VQSort":11,"RUNE_v6":3,"RUNE_v5":3,"RUNE_v7":1}`.

- [Raw x86-64 rows](x86_amd_epyc_7763_raw.csv)
- [x86-64 median table](x86_amd_epyc_7763_summary.csv)

## Robust results and counterexamples

1. **Replicated, narrowly scoped gain:** RUNE v7 beats frozen v6 and VQSort at K40960, K49152, K65536 and K73728 across both CPUs at N=1M in the recorded seed families.
2. **Magnitude differs by CPU:** Larger v6-to-v7 speedups occur on x86-64, but the new tier remains useful on ARM64. This suggests hardware-dependent fallback performance, not a universal multiplier.
3. **Capacity boundary failure:** At K81920, RUNE v7 loses to v6 on both processors. x86-64 RUNE v7 is far slower than VQSort (24.04ms versus 13.40ms). Similar fallback penalty at K90112; the 16K-sample duplicate pilot needs better calibration or adaptive rejection.
4. **ARM portability limitation:** Without x86 BMI2 acceleration, sparse16 runs near 18.6ms on ARM64 and sequential IPS4o around 15.6ms. On x86, RUNE's compact-variation-bit implementation is substantially faster.
5. **High entropy:** VQSort dominates x86-64 uniform random64 (12.92ms versus RUNE v7 26.47ms); ARM64 RUNE is slightly faster than its local VQSort, which shows architecture-specific SIMD/algorithm trade-offs rather than universal rankings.
6. **Small N:** At N=100K, VQSort wins 11/18 workloads on x86-64; RUNE's frequency setup cost and dispatch impose crossovers.

## Evidence standard and next intervention

These are empirical, CI-executed performance claims, not machine-verified proofs or novelty claims. Results show non-universal, distribution-specific optimization; they cannot establish overall SOTA.

Next discriminating test: **K73K–K90K boundary acceptance** with held-out seeds and a calibrated two-stage sample pilot, while guaranteeing rejected table scans leave input unchanged. Also investigate ARM-specific sparse16 bit-compression costs, and benchmark against CAFS on x86 when considering any new dispatch.

No merge of RUNE v6 PR #1, v7 PR #2, or this cross-arch PR without user direction.
