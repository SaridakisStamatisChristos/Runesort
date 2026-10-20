# RUNE v13 — ARM64 mask-topology-aware radix dispatch

**Research verdict: CONDITIONAL GO for the narrow ARM64 optimization; HOLD merge.**

This is an evidence-backed workload-specific optimization, **not a universal state-of-the-art claim, not a new radix-sort algorithm, and not a proof of novelty**. Keep this research PR draft and separate from `main`. Frozen v7, v10–v12, proprietary LICENSE and the default released v5 remain unchanged.

## Problem contract

- Sort arrays of unsigned `uint64_t` values in ascending order; API is not a stable record sort.
- C++17 core; single thread; no external sort dependency in the production candidate.
- Primary objective: reduce ARM64 cost of sparse, noncontiguously varying bit masks (17–23 bits) at moderate/large `n` without regressions on contiguous or smaller-domain inputs.
- Baseline is frozen RUNE v7. External competitors are pinned Google Highway VQSort and sequential IPS4o. No pooling across CPUs or CI runs.

## Mechanism

Frozen v7 uses hardware `_pext_u64` on BMI2-enabled x86, but otherwise extracts each selected bit in a loop. Its packed-digit radix may call this loop on every key during both histogram accumulation and stable scatter. For widely distributed sparse bit masks on ARM64, this software extraction becomes costly.

RUNE v13 computes a **complete** variability mask during the inherited array scan. After the existing numeric-range counting opportunity:

1. Only on `__aarch64__`, `n >= 262144`, and 17–23 varying bits, consider byte-radix dispatch.
2. For 17–22 varying bits, examine the same successive up-to-11-bit masks that packed radix would generate. Switch to the existing `dense_radix` only if a digit is **noncontiguous** and would invoke software extraction. Otherwise retain the old v7 path.
3. For 23 varying bits, prefer `dense_radix` on the tested ARM64 regime. This part remains an empirical heuristic, not a universal performance rule.
4. No runtime behavior changes on x86: the conditional path and predicate are guarded by `#if defined(__aarch64__)`.

**Correctness rationale:** every byte marked invariant by the complete mask is constant over the input. The reused stable least-significant-byte-first radix sorts all remaining bytes in numeric significance order. Thus all unsigned 64-bit keys are ordered exactly, independent of sample heuristics. The selection criterion affects time, not output. Auxiliary memory is O(n); with fixed 64-bit keys the byte-radix portion is O(8n + 8·256), not an asymptotic advance over known radix methods. Overall fallback complexity retains the existing v7 bounds.

## Verified implementation identity

- Final sorter-source change: `1f7dd69074adae72591df0c734613fa0eb4e45d2`. Later commits alter workflows or evidence only; sorter header is unchanged.
- [Final native ARM64/x86 builds and comparisons](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39430115651): **four of four jobs passed** including ASan/UBSan on both architectures, portable x86 no-BMI2, and 20/20 native CTests on each architecture. Direct sanitizer test executed 252 adversarial/boundary cases per runner.
- [Final topology expansion, dual architecture](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39430191804): **two of two jobs passed**; contiguous, clustered, staggered, spread, and independent random masks.
- [Final pinned external four-way comparison](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39430191859): **two of two jobs passed**, comparing RUNE v13, frozen v7, Google Highway VQSort, and sequential IPS4o. Comparator sources were fetched at immutable revisions, not incorporated into RUNE.
- **1660/1660 benchmark result rows matched independent `std::sort` output**, across native (540), topology (480), and external (640) records.
- GCC 13.3, Ubuntu 24.04. ARM64 native Neoverse-N2; native x86 AMD EPYC 7763; external x86 AMD EPYC 9V45. These are virtualized GitHub runners, not dedicated physical CPU trials.
- Native and external input seeds were independent; each workload/CPU run used five input seeds, one warmup and four measured repetitions, randomized algorithm order, identical generated inputs, and copying/oracle outside sort timings.
- Per-workload time in this report is median of five seed-level median runtimes. Speedups in machine-readable reports are median same-seed time ratios; they need not equal the ratio of medians.

## Native comparison, ARM64, n = 1 million

| Workload | v7 ms | v13 ms | Ratio of reported medians (approx.) |
|---|---:|---:|---:|
| sparse17 | 33.378 | 15.828 | 2.11× |
| sparse20 | 38.836 | 15.815 | 2.46× |
| sparse22 | 43.238 | 15.969 | 2.71× |
| sparse23 | 45.988 | 15.976 | 2.88× |

All figures are native same-run comparisons. ARM64 four-million-element results similarly show large improvements; detailed rows and the exact paired-ratio summaries are in `native_summary.csv`.

## External competitors, ARM64, n = 1 million

| Workload | v13 ms | VQSort ms | IPS4o ms | Result |
|---|---:|---:|---:|---|
| sparse17 | 16.065 | 18.013 | 17.768 | v13 fastest of these |
| sparse20 | 15.881 | 17.427 | 18.983 | v13 fastest |
| sparse22 | 16.223 | 17.330 | 18.952 | v13 fastest |
| sparse23 | 16.032 | 17.277 | 18.957 | v13 fastest |

At n=4M, v13 also beats VQSort on sparse20, sparse22, sparse23 on this ARM runner, but VQSort and IPS4o beat v13 on sparse17. Both external CPU jobs validated exact equality to std::sort; this is a **bounded workload-specific win**, not universal superiority.

## Falsification and repair

The first guard routed all 16–23-bit masks through dense radix, then the first ARM-native qualification showed a ~13% sparse16 regression at n=4M. Restricting to 17–23 eliminated that regression.

A separate mask-topology adversarial study showed the second guard still caused ~12% and ~15% regressions on contiguous 17- and 20-bit masks at n=4M. These are real counterexamples, not discarded outliers.

The final guard checks whether packed digits actually need costly bit extraction and preserves v7 when all needed digits can be evaluated as contiguous shifts (17–22-bit region). The final topology requalification found:

- Contiguous17-high: paired changes about −0.12% at 1M and −0.46% at 4M.
- Contiguous20-mid: about −0.47% at 1M and −0.34% at 4M.
- Randomly positioned 20-bit masks: approximate 2.62× runtime ratio at 1M, 2.32× at 4M (from paired medians).
- 20 varying bits in three separated byte groups: **4.70× paired runtime ratio** at 1M, with large 4M gains.
- Globally contiguous 23-bit masks were faster on the tested machines, but this 23-bit policy needs more independent mask placements and CPUs.

Thus the revised mechanism survived the tested topological counterexamples without demonstrated material ARM64 regressions in those cases. This is **empirical evidence, not a mathematical proof of optimal dispatch**.

## Negative results and limitations

- x86 still has the unchanged v7 algorithmic path and can show apparent timing variation from code placement, scheduler, VM placement or compilation. Do not attribute those variations to the ARM64-only branch.
- Pinned VQSort remains much faster on many x86 full-width/high-cardinality inputs. At n=4M on ARM64, IPS4o is faster on sparse17 and K81920. On ARM64 sparse16, RUNE v13 retains the old path and external sorters still have advantages.
- Hardware and VM noise, mask-topology family selection, insufficient independent physical hosts, input-generation assumptions, and limited seeds prevent extrapolating these results to every deployment.
- Prior work on radix sort, in-place radix distribution, and vectorized quicksort covers the general mechanism classes. This implementation makes no global novelty or fastest-sorter assertion. See [Google's VQSort paper](https://research.google/pubs/vectorized-and-performance-portable-quicksort/) and [IPS4o's repository](https://github.com/ips4o/ips4o).

## Evidence ledger

- RUNE v13 functional correctness (tested workloads): **EMPIRICALLY_VALIDATED**, not formally verified.
- Full-array mask and stable byte-radix correctness reasoning: **ANALYZED**, complete algorithm-wide proof not claimed.
- ARM64 target performance: **EMPIRICALLY_VALIDATED in the tested VM workload regimes**, with two independent seed sets and adversarial mask-shape checks.
- Global/portable SOTA: **NOT ESTABLISHED**.
- Novelty: **PRIOR ART FOUND** for component mechanisms; no novel asymptotic result.

## Artifacts

- [Native ARM64 raw trials](native_arm64_neoverse_n2_raw.csv), [native x86 raw trials](native_x86_epyc7763_raw.csv), [native summary](native_summary.csv)
- [ARM64 topology raw trials](topology_arm64_neoverse_n2_raw.csv), [x86 topology raw trials](topology_x86_epyc7763_raw.csv), [topology summary](topology_summary.csv)
- [ARM64 four-way external raw trials](external_arm64_neoverse_n2_raw.csv), [x86 four-way external raw trials](external_x86_epyc9v45_raw.csv), [external summary](external_summary.csv)

## Next gates before release or merge

1. Replicate on dedicated ARM64 hardware with performance counters, memory budget profiles, frequency control and random mask-placement seeds. Check the exact threshold n=262143/262144 and varied-bit 16/17/22/23/24 boundaries again.
2. Isolate expensive software extraction against the improved byte-radix worker with hardware profiles; benchmark shorter n and broader payload arrangements.
3. Independently compare with previously promising ARM sparse16 exact-frequency candidate [PR #5](https://github.com/SaridakisStamatisChristos/Runesort/pull/5) without integrating unverified changes.
4. Only after explicit owner approval, consider a minimal cherry-pick or targeted release branch; preserve this research ledger and proprietary licensing.

**Decision: CONDITIONAL GO for continued targeted engineering evaluation. HOLD MERGE.**
