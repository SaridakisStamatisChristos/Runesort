# RUNE v17 — eight-pass scratch allocation experiment

**Research only — HOLD MERGE.** Frozen comparator: v16c source from PR #16, base commit `220313750c9a96e2250a29e9e41ed1b81bb9d47a`. Main, v14/v16c implementations, archival research and proprietary LICENSE are untouched.

Only `detail::fixed_radix<8>` uses RAII scratch without value initialization. `fixed_radix<4>`, `dense_radix`, all cardinality gates, reconstruction tables and dispatch are unchanged. Histogram prefix sums partition [0,n); each stable LSD scatter fills all destination positions before reads. O(n) auxiliary memory and eight distribution passes are unchanged.

**Prior art/internal negative evidence:** [v10 PR #9](https://github.com/SaridakisStamatisChristos/Runesort/pull/9) tested the same initialization optimization on v7 for both fixed and dense radix, showing small gains and some losses. This v17 study tests a **narrower v16c-specific backport**, not novelty. Failed v15 six-pass radix is not revived.

Qualification: CMake regression tests, explicit raw fixed8 worker vs production v17/frozen v16c/std::sort equality (including 81,920 and 85,000 K at N=2M), ASan/UBSan and portable no-BMI2 on x86, five-seed exact same-run paired benchmarks on x86-64/ARM64, N=1M/4M with seven positive/negative-control regimes. One warmup plus five randomized-order trials. Raw CSV artifacts retained by CI.

**Decision rule:** No performance claim until native jobs complete; no promotion without convincing repeatable target improvement and no material control regression. Virtual CPUs and small effects limit confidence. Global SOTA not claimed.
