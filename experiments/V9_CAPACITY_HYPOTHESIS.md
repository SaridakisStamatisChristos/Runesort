# RUNE v9 hypothesis: exact 131K-key frequency tier

**Research-only ablation, not an algorithm release.** Branched from frozen RUNE v7. Neither the v7 sorter, any previous research PR, `main`, nor the proprietary LICENSE is changed.

RUNE v8's extra sample gate was correct but failed its **net sort-performance** acceptance criterion; see [negative report on v8](https://github.com/SaridakisStamatisChristos/Runesort/blob/research/rune-v8-two-stage-boundary-20261020/runesort_v8/evidence/EXTERNAL_BENCHMARK_VERDICT.md). The remaining K≈82K cliff may arise from RUNE v7's fallback.

Test two **exact fail-closed** frequency tables, both with a 131,072-key capacity and 32-probe limit, using 262,144 versus 524,288 hash slots. Each rejects without mutating the input; when it rejects, the benchmark times **both the failed attempt and the frozen v7 fallback**. Data copy/generation/oracle are outside the timing. Scratch allocations and the unique-key pair sort are included.

Compete on same-run native x86/ARM64 with frozen v7 and Boost pdqsort. Cases at N=200K and N=1M: K32K..180K, uniform random64 and sparse16. Four new independent seeds, three repetitions after one warmup; every output checked against std::sort. The benchmark deliberately checks that arrays with >131K distinct values are unchanged after rejected reconstruction.

**Acceptance:** Require meaningful improvements over frozen v7 at K81,920, K90,112 and K98,304 on **both** architectures without a large regression at K147K/K180K or uniform random. At least one table must justify the added ~3.25MB or ~6.5MB initialized memory plus sorting unique pairs. This ablation **does not** measure VQSort in the same run, so no VQSort superiority may be claimed.

No change or merge to the production sorter unless this causal ablation survives correctness and performance gates.
