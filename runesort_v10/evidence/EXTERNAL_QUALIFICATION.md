# RUNE v10: independent external-sorter qualification

**Decision:** targeted fallback candidate SURVIVES, but **NO-GO for overall SOTA or immediate merge**. Scope remains a narrow optimization of RUNE v7's full-width radix fallback; there is no new algorithmic novelty claim.

## Reproducible evidence and provenance

- [Nine-algorithm dual-CPU CI run](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39399873667) — **both jobs succeeded** and every sorted output matched a separate std::sort oracle.
- **Exact benchmarked sorter source:** commit `150d57088cf7d08a9cc11540bd9d1c2c6ce8abca`. Subsequent additions to the branch contain documentation and raw evidence only. The same guarded radix algorithm passed [full CMake CI](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39399873617).
- Native GitHub Ubuntu 24.04 `x86-64 AMD EPYC 7763` and `ARM64 Neoverse-N2`; GCC 13.3, `-O3 -march=native -DNDEBUG -std=c++20`. CPUs ran independently, no cross-CPU runtime ratios.
- Nine exact ascending uint64 sort implementations per machine: frozen RUNE v10/v7/v6/v5, pinned Highway VQSort, pinned sequential IPS4o, Boost pdqsort/spreadsort and `std::sort`.
- Highway revision `c3407ccaaecffa735c911cae3971efa02263211d`; IPS4o revision `8d5a97bb4c4048a1eef18b537f191a31462b051a`. **CAFS is not in this cross-architecture benchmark**, because it requires x86 features; earlier parent v7 x86 CAFS results are not comparable across new runner timings.
- Two sizes N=100,000 and N=1,000,000; 17 distributions including K=81920–200000. Five **additional held-out seeds** not used by the earlier v10-v7 direct test, one warmup and four measured repetitions per seed. Per-seed median of four replicates followed by median across five seeds.
- Sorting time excludes input generation/copy/oracle comparison and includes sorter-owned allocations. Randomized algorithm call order, exact same input for all algorithms within each case and seed. No CPU pinning, cache-counter attribution, clock stabilization, or confidence intervals. Time comparisons are empirical, not formal proof.

## X86-64 AMD EPYC 7763, N=1M

| Workload | v10 ms | v7 ms | VQSort ms | Fastest |
|---|---:|---:|---:|---|
| uniform64 | 29.947 | 29.842 | 12.936 | VQSort |
| random32 | 20.557 | 20.279 | 13.077 | VQSort |
| sparse16 | 5.728 | 5.482 | 13.764 | RUNE_v6 |
| K73728 | 12.670 | 12.706 | 13.612 | RUNE_v10 |
| K81920 | 22.560 | 25.702 | 13.647 | VQSort |
| K90112 | 23.145 | 25.708 | 13.532 | VQSort |
| K98304 | 23.390 | 26.625 | 13.594 | VQSort |
| K114688 | 25.791 | 30.824 | 13.514 | VQSort |
| K131072 | 26.046 | 32.666 | 13.486 | VQSort |
| K200000 | 30.657 | 32.469 | 13.349 | VQSort |

Winner distribution across **17** one-million-key workloads: `{"VQSort":9,"RUNE_v6":2,"RUNE_v10":5,"RUNE_v7":1}`; at N=100K: `{"VQSort":13,"RUNE_v7":1,"RUNE_v10":3}`.

- [x86 raw 1530 measurements](x86_epyc7763_external_raw.csv)
- [x86 aggregated summary](x86_epyc7763_external_summary.csv)

## ARM64 Neoverse-N2, N=1M

| Workload | v10 ms | v7 ms | VQSort ms | Fastest |
|---|---:|---:|---:|---|
| uniform64 | 15.332 | 15.333 | 17.335 | RUNE_v5 |
| random32 | 11.963 | 11.389 | 17.374 | RUNE_v7 |
| sparse16 | 18.849 | 18.493 | 17.850 | IPS4o_seq |
| K73728 | 11.543 | 11.595 | 18.132 | RUNE_v10 |
| K81920 | 12.491 | 13.104 | 18.089 | RUNE_v10 |
| K90112 | 12.397 | 12.803 | 18.059 | RUNE_v10 |
| K98304 | 12.424 | 12.702 | 18.056 | RUNE_v10 |
| K114688 | 12.382 | 12.559 | 18.075 | RUNE_v10 |
| K131072 | 12.223 | 12.355 | 18.022 | RUNE_v10 |
| K200000 | 12.278 | 12.370 | 17.891 | RUNE_v5 |

Winner distribution across **17** one-million-key workloads: `{"RUNE_v5":2,"RUNE_v7":2,"IPS4o_seq":1,"RUNE_v6":3,"RUNE_v10":9}`; at N=100K: `{"RUNE_v5":6,"VQSort":1,"RUNE_v7":4,"RUNE_v10":4,"RUNE_v6":2}`.

- [ARM raw 1530 measurements](arm64_neoverse_n2_external_raw.csv)
- [ARM aggregated summary](arm64_neoverse_n2_external_summary.csv)

## Strong findings and falsification

1. **Credible, bounded v7 improvement:** On x86 EPYC 7763 at K81920–K147456, the guarded 10-bit radix path cut medians by roughly 10–21% compared with v7. On ARM Neoverse-N2, it cut medians by about 1–5%. Five held-out seeds and same-run frozen v7 comparisons support a repeatable but hardware-dependent benefit.
2. **NOT competitive with VQSort on x86 high-cardinality:** At K131072, v10 26.05ms versus VQSort 13.49ms, despite improving v7 32.67ms. VQSort also wins uniform64 and most 100K cases on EPYC 7763. A candidate that improves v7 still need not be overall fastest.
3. **ARM reverses the relative ranking:** At K131072, RUNE v10 12.22ms versus VQSort 18.02ms. This is architecture-specific performance, not a contradiction or global rank.
4. **Potential collateral regressions:** On ARM random32, v10 11.96ms versus v7 11.39ms (~5% worse); sparse16 18.85ms vs 18.49ms. On x86 sparse16 v10 5.73ms vs v7 5.48ms (~4.5% worse). These should not be ignored just because the guarded branch is not expected to execute: compiler layout and noisy shared runners can change measured times, and causality remains unverified.
5. **Memory:** v10's seven radix histograms occupy ~56 KiB of counters versus v7's ~16 KiB for eight byte-radix histograms. Scratch remains N×8 bytes. Full radix is stable; public RUNE only sorts unsigned 64-bit values, not arbitrary stable records.
6. **Prior art:** Wider-digit LSD radix and fused-histogram approaches are established; this is an implementation refinement only. Research references: `eloj/radix-sorting`, `clausecker/radsort`, `klauspost/radsort`.

## Research gate
- **State:** EMPIRICALLY_VALIDATED for correct output and a **narrowly qualified performance improvement over v7** on tested K~82K–147K input distributions. Not a proof and not global SOTA.
- **Recommendation:** Keep **draft** pending a dedicated-hardware run, cache/memory counters, larger N sweep, out-of-distribution inputs and investigation of random32/sparse16 negative results. If merging later, merge stacked v7 first with explicit user decision.
- All observed negative variants are documented in [candidate bank](CANDIDATE_BANK.md); v8 and v9 NO-GO branches remain separate, and `main` + proprietary LICENSE remain unchanged.
