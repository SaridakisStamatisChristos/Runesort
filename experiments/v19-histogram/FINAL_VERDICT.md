# RUNE v19 — completed compact-histogram qualification

**Decision: NO-GO for production integration; conditional mechanism-only interest in fused32, reject perpass32. HOLD MERGE.**

Frozen baseline: v17 implementation commit `7c87921c30217d4317a964fed07debcc16b2f183` (source present at branch parent `927d29b778b75f67e8a908cffa914a5f6e88929f`). Qualified experiment commit `1bd17bb5d6669040bb476fb424ba18c6e19e3f28` (**6/6 CI jobs passed**).

## Evidence

- [Dedicated x86 and ARM exact native CI + sanitizers](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39488183226): 4/4 passed.
- [Existing push CI](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39488182919): passed.
- [Existing pull request CI](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39488235807): passed.
- **300 of 300 measured full-array oracle runs correct** (150 x86 + 150 ARM). Both ASan/UBSan jobs passed, x86 `-mno-bmi2` passed and native CMake/CTest passed.
- Compiler C++17, `-O3 -march=native -DNDEBUG`, GitHub Ubuntu 24.04 x86/ARM virtual hosts; 5 held-out seeds (82103, 82111, 82117, 82129, 82133), 1 warmup, 3 timed randomized-order trials per algorithm, N=1M/4M. Each candidate sorts the same input values and is checked against an independent std::sort oracle.
- The 4M uniform64 target compares the **direct fixed8 worker**, not end-to-end public RUNE sort. No pinned VQSort/IPS4o benchmark executed in this v19 experiment.

## Native 4M results (milliseconds, lower is better)

| CPU | Distribution | Frozen v17 | Fused 32-bit | Per-pass 32-bit | Fused ratio (wins) | Per-pass ratio (wins) |
|---|---|---:|---:|---:|---:|---:|
| x86_64 | uniform64 | 62.768 | 62.602 | 69.455 | 1.003× (4/5) | 0.904× (0/5) |
| x86_64 | random32 | 52.378 | 51.815 | 57.117 | 1.011× (5/5) | 0.916× (0/5) |
| x86_64 | K85000 | 63.488 | 62.866 | 74.621 | 1.009× (4/5) | 0.850× (0/5) |
| x86_64 | sorted | 63.673 | 62.810 | 69.977 | 1.009× (4/5) | 0.909× (0/5) |
| x86_64 | alternating | 43.089 | 42.789 | 47.836 | 1.007× (5/5) | 0.899× (0/5) |
| arm64 | uniform64 | 80.767 | 79.970 | 90.700 | 1.014× (3/5) | 0.890× (0/5) |
| arm64 | random32 | 92.443 | 89.521 | 110.247 | 1.015× (3/5) | 0.837× (0/5) |
| arm64 | K85000 | 72.719 | 71.141 | 91.991 | 1.021× (5/5) | 0.791× (0/5) |
| arm64 | sorted | 81.070 | 81.430 | 90.856 | 0.995× (2/5) | 0.889× (0/5) |
| arm64 | alternating | 109.301 | 107.119 | 128.662 | 1.047× (3/5) | 0.823× (0/5) |

Displayed latencies are medians of per-seed median observations; ratio columns are separately medians of seed-matched baseline/candidate ratios, not ratios of displayed latency medians.

## Hypothesis tests

1. **Compact fused `uint32_t[8][256]`:** 8 KiB rather than baseline 16 KiB, exact semantics for N<=UINT32_MAX, fallback to the frozen worker otherwise. On x86 at N=4M uniform64, baseline 62.768 ms versus fused32 62.602 ms (around **0.3%**), 4/5 seeds. ARM uniform64 80.767 ms versus fused32 79.970 ms, 3/5 seeds. Gains are too small or insufficiently replicated on controlled physical hosts to support integration.
2. **Per-digit counting `uint32_t[256]`:** 1 KiB working histogram but eight extra full input scans. X86 at N=4M uniform64 is 69.455 ms vs baseline 62.768 ms; ARM is 90.700 vs 80.767. **Perpass32 rejected** for tested full-width regimes, worse on all 5 uniform64 seeds on both machines.
3. **Sorted / alternating controls** force a full radix kernel. Production RUNE has early exits or other choices; these controls are *not* a dispatch performance claim.
4. Both tested mechanisms are standard radix implementation strategies; **no algorithmic novelty or SOTA** claim. The prior v18 buffered scatter failure is separate and remains archived unmerged.

## Raw evidence

- [x86 per-seed timings](x86_64_raw.csv) — 150 rows
- [ARM per-seed timings](arm64_raw.csv) — 150 rows
- [paired_summary.csv](paired_summary.csv) — 20 CPU-size-workload groups
- [provenance.json](provenance.json) — frozen references, exact commands, CI run IDs

## Next highest-value research

Stop implementing per-pass hist. Do **not** merge fused32 based on ~0.3% x86 target gain. Use actual physical hardware or repeat independent runs with perf/PMU counters when authorized. Diagnose x86 full-width spread, include a pinned Highway VQSort and IPS4o same-host comparator, and only then select another causal mechanism. Initial smaller hist experiment can be revisited if a repeatable >3% improvement emerges across runners and no important workload regresses.
