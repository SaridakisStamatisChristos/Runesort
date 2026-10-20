# RUNE v11 — Compacted scratch experiment: native cross-architecture verdict

**Decision: NO-GO for promotion.** Keep this as an archived experimental draft. This change is correctness-tested but has no robust cross-architecture performance advantage. Do not merge into the v10 parent or main.

## Reproducible provenance

- Tested source commit: `0af8af82c478cbb4565888d6602bc35427e6483d`. Post-run evidence commit contains no sorting implementation changes.
- [Full native benchmark/sanitizer workflow](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39424573451): **all four jobs succeeded** (ARM64 and x86 benchmarks; ARM64 and x86 ASan/UBSan). Portable x86 no-BMI2 test succeeded.
- [Repository CI build and full CTest run](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39424573331): **passed**; native jobs separately ran **25/25 tests on each architecture**.
- Hardware: native GitHub Actions ARM64 **Neoverse-N2**; native x86-64 **AMD EPYC 9V45**; Ubuntu 24.04, GCC 13.3.0, `-std=c++17 -O3 -march=native -DNDEBUG`.
- Benchmark: 12 workloads, n=1,000,000 and 4,000,000, five independent fixed seeds, one warmup and four measured repetitions per seed; randomized version order and identical input per version. Sorting time excludes copy/oracle. Every one of the **720 result rows** reports an exact match to `std::sort`.
- Each runtime below is the **median across five per-seed median runtimes**, in ms. Each percentage uses the **median of five same-seed v10/v11 ratios**. These two statistics need not be algebraically identical.
- Only compare versions within the same CPU job; VM noise, CPU placement, thermal variability, and the 5-seed sample limit inference.

## Target sparse-bit cases — v10 → v11 (ms; paired change)

| CPU | n | sparse16 | sparse20 | sparse22 | sparse23 |
|---|---:|---:|---:|---:|---:|
| EPYC 9V45 | 1M | 4.39 → 4.34 (+1.91% paired median) | 7.30 → 8.04 (-7.03% paired median) | 8.44 → 8.64 (-4.44% paired median) | 9.27 → 10.20 (-5.35% paired median) |
| EPYC 9V45 | 4M | 17.01 → 16.43 (+2.36% paired median) | 33.33 → 32.31 (+2.95% paired median) | 41.55 → 40.41 (+3.15% paired median) | 49.22 → 47.69 (+2.48% paired median) |
| Neoverse-N2 | 1M | 18.29 → 18.29 (+0.00% paired median) | 38.92 → 38.91 (+0.07% paired median) | 43.35 → 43.35 (-0.07% paired median) | 45.57 → 45.60 (-0.03% paired median) |
| Neoverse-N2 | 4M | 68.58 → 68.63 (-0.10% paired median) | 156.57 → 156.27 (+0.10% paired median) | 175.00 → 174.79 (+0.12% paired median) | 187.30 → 187.35 (-0.03% paired median) |

## Interpretation

- **x86 n=1M:** sparse20, sparse22, sparse23 regress on paired medians; the worst is sparse20 (~7% slower). This is incompatible with a blanket claim that avoiding scratch initialization is faster.
- **x86 n=4M:** sparse20/22/23 improve by roughly 2.5–3.2% on paired medians. The direction flips between these tested input sizes.
- **ARM64:** sparse20/22/23 show changes near measurement noise (<0.2% paired medians) at both sizes. No convincing portable gain.
- High-entropy and bounded-cardinality controls are mixed; see full machine-readable summary. This optimization does not close the broader gap to Highway VQSort on difficult generic domains, and no external-sorter benchmark was run for v11.
- **Mechanism:** `new uint64_t[n]` omits value initialization; histogram prefix ranges still guarantee complete scratch writes before reads. A plausible, **untested** explanation for the x86 sign reversal is OS page first-touch/TLB/cache behavior: clearing scratch sequentially can pre-fault or pre-touch pages before scattered writes. This is a hypothesis, not a demonstrated causal mechanism.

## Evidence states

- Exact unsigned 64-bit ordering over the executed differential suite: **EMPIRICALLY_VALIDATED** (not a formal correctness proof).
- The full-write scratch safety invariant: **ANALYZED**, with executed address/undefined-behavior sanitizer support.
- Universal or portable speedup hypothesis: **REFUTED in the tested regimes**, given meaningful x86 regressions and near-zero ARM effects.
- Global fastest-sorter or algorithmic-novelty claim: **NOT ESTABLISHED**. This is a standard memory-initialization implementation technique.

## Next controlled experiment

Do not hard-code an n-threshold from these two sizes. Before a new version, isolate page-fault placement using a controlled pre-touch-per-page candidate, broader logarithmic N-sweep (e.g. 256K to 8M), matched fresh seeds, CPU affinity, page-fault counters and cache/TLB profiling on stable physical hardware. Only promote a mechanism that holds without important regressions.

## Raw evidence

- [AMD EPYC x86 raw CSV](x86_amd_epyc_9v45_raw.csv)
- [Neoverse-N2 ARM64 raw CSV](arm64_neoverse_n2_raw.csv)
- [Summary of paired five-seed comparisons](native_summary.csv)

No source changes to v7, v10, or `main`, and no license modifications.
