# RUNE v15 11-bit radix — experimental NO-GO

## Hypothesis
Replace x86 high-entropy eight-pass byte radix with a six-pass (11,11,11,11,11,9) stable LSD radix. The **new worker is exact**, counts 64-bit keys in 6 bucket histograms and uses an uninitialized scratch buffer, fully written on each pass. Complexity: fixed-width O(n), O(n) scratch, plus 98,304-byte stack histogram. Prior art: radix LSD counting; not novel asymptotics.

## Execution status
- [Pinned external x86/ARM64 VQSort/IPS4o/native benchmark](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39451780219) **2/2 SUCCESS**, 1,080/1,080 exact output comparisons, 9 workload families × 2 sizes × 5 independent seeds × 6 algorithms, four timed repeats after warmup; GCC 13.3 `-O3 -march=native`. External comparator source pinned as in the v14 experiment.
- [Direct mechanism isolation on AMD EPYC 7763](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39452256776) **SUCCESS**, 40 exact-output result rows, each with 4 measured repeats and randomized order.
- [Native ASan + UBSan including portable x86 no-BMI2](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39451980821) **2/2 passed**, direct worker and v15 dispatcher adversarial tests.
- CMake repository and v15 tests on x86 passed.

## Contradictory performance: full-width random64 only (median of five seed medians)

| Processor and sample size | Frozen v14 | v15 | Pinned VQSort |
|---|---:|---:|---:|
| AMD EPYC 7763, 1M (mechanism-only separate run) | 26.120 ms | 15.424 ms | not timed |
| AMD EPYC 7763, 4M (mechanism-only separate run) | 106.969 ms | 100.213 ms | not timed |
| AMD EPYC 9V45, 1M (same-run external) | **6.090 ms** | 8.802 ms | **3.243 ms** |
| AMD EPYC 9V45, 4M (same-run external) | **29.194 ms** | 68.546 ms | **16.089 ms** |

The massive negative **4M** result is decisive; do not promote this branch over v14. The AMD EPYC 7763 microbenchmark demonstrates that pass reduction can help on some processors, but another processor's much larger histogram penalties and memory-scatter costs dominate. A CPU-specific heuristic is not justified by these two isolated virtualized hosts; would require performance counters, isolated core, controlled clock and repeated hardware families.

ARM64 production dispatcher remains identical to v14 apart from inactive x86-only code and code placement; external Neoverse-N2 timing differences are generally small and no gain is established.

## Root-cause model and next experiment

The eight-pass radix has 8 × 256 histogram bins (~16 KB), whereas v15 has 6 × 2,048 (~96 KB). **Hypothesis only:** the latter incurs expensive histogram cache misses, memory scattering / TLB effects; fewer streaming passes do not necessarily outweigh that. No hardware counters were recorded, so do not assert this as a proved diagnosis.

The broader completed gauntlet reveals a separate high-value *dispatch* problem at high-cardinality boundary K81,920. The fifth-tier 16,384-sample repeat pilot requires >=1,600 repeats; for a uniform palette of K=81,920, expected duplicate samples are approximately **1,534**, so the pilot often rejects a valid bounded-frequency route. This is an independent, falsifiable next experiment requiring no radix worker replacement.

**Verdict: v15 NO-GO for deployment or release, preserve original v14, do not merge.**
