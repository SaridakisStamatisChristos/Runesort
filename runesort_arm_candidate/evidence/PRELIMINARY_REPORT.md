# RUNE ARM64 sparse-bit dispatch — executed early evidence

**Status:** Direct same-input comparison passed on Neoverse-N2 and AMD EPYC 7763; full nine-way VQSort/IPS4o comparisons pending.

- [Completed initial runner matrix](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39385296411)
- [Passed CMake build and differential tests](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39385296374)
- GCC 13.3 C++17, `-O3 -march=native -DNDEBUG`, unsigned `uint64_t` sorted ascending.
- Input clones excluded from timed region; exact matching with `std::sort` on every run. Four seeds, four repetitions after warmup; rotating algorithm order. Data includes duplicate outlier16 instances with different mode indices; preserve them rather than conflating them into one seed.
- Algorithm change uses full-array bit variability `varied==16` on ARM64 only, after numeric-range tests; exact existing fifth frequency engine, fail-closed on bounded rejection. x86 code paths compile unchanged.

| Machine | frozen v7 sparse16 | candidate sparse16 | candidate/v7 runtime |
|---|---:|---:|---:|
| Neoverse-N2 ARM | 18.342ms | 13.492ms | 0.736x |
| EPYC 7763 x86 | 6.850ms | 6.675ms | 0.974x |

**Caveats:** This is a performance specialization; does not validate novelty, global SOTA or across-machine superiority. CPU noise causes small changes on unchanged x86 paths; only the larger ARM sparse16 difference supports a mechanism. More workloads and same-run VQSort/IPS4o are required before promotion.

**License and branches unchanged:** this experiment has not merged into main; proprietary license preserved.
