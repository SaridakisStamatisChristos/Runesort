# RUNE v14 — Comprehensive sorting benchmark (20 October 2026)

## Decision: full 6/6 CI experiment completed; global SOTA NOT established

**Final primary source:** [GitHub Actions workflow run 39440290337](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39440290337), from tested benchmark source commit `1f72d606e21f865af3d30e81b4a66e99ec459094`. All six native Linux competition jobs **passed** and all their CSV-completeness and exact-result validators passed.

**All 7,425 benchmark measurement rows were checked against independently computed `std::sort` output**: 3,861 x86 rows, 3,564 ARM64 rows. Each measurement row is the median of 3 timed repetitions of the same algorithm and input after a separate warmup; 3 independent input seeds per workload. This amounts to 22,275 timed sorting calls plus 7,425 warmup calls, all correctness-checked. No number in this report represents synthetic timings.

This is a **representative broad comparison of important accessible single-threaded unsigned-64-bit sorters**, **not literally every sorting algorithm in existence** and not a mathematical theorem or performance proof. Run-to-run variation, hardware differences, and unbalanced specialized workloads prohibit a universal top-sorter claim.

## Algorithm roster (genuine implementations)

Six frozen internal implementations:
1. RUNE v14 (candidate)
2. RUNE v13
3. ARM sparse16 standalone experimental variant (its x86 behavior is inherited v7)
4. RUNE v7
5. RUNE v6
6. RUNE v5

Six external comparators on both architectures:
7. Google Highway **VQSort**
8. Sequential **IPS4o**
9. Boost **pdqsort**
10. Boost **spreadsort**
11. GNU C++ **std::sort**
12. GNU C++ **std::stable_sort** (provides additional stability semantics, not required here)

Thirteenth comparator on **supported x86 only**:
13. **CAFS**, using Boost pdqsort as its fallback, requiring AVX2 and BMI. Do **not** extrapolate CAFS results onto ARM64; it is not implemented there in the pinned repository. No comparison with GPU, parallel, or key/payload-record sorting is claimed.

External source commits:
- [Highway](https://github.com/google/highway/commit/c3407ccaaecffa735c911cae3971efa02263211d) `c3407ccaaecffa735c911cae3971efa02263211d`
- [IPS4o](https://github.com/SaschaWitt/ips4o/commit/8d5a97bb4c4048a1eef18b537f191a31462b051a) `8d5a97bb4c4048a1eef18b537f191a31462b051a`
- [CAFS](https://github.com/kexibq-official/cafs-lib/commit/13488afef446ae88e7ad631ec6f862def896094a) `13488afef446ae88e7ad631ec6f862def896094a`
- Boost: Ubuntu 24.04 runner `libboost-dev` installed via apt; distribution-version updates are a remaining reproducibility limitation.

## Input contract and methodology

- C++20 GCC 13.3 on Ubuntu 24.04, `-O3 -march=native -DNDEBUG -pthread`; one sort thread. `uint64_t` ascending, value-only comparison. Arrays of N=100,000, 1,000,000, and 4,000,000.
- 33 input modes in three groups: 12 entropy/sparse-bit distributions; 12 key-palette/cardinality distributions; 9 partially ordered/adversarial distributions.
- Three independently seeded inputs per mode: base seeds `631003, 631013, 631021` plus fixed per-mode offset. Algorithm order randomized each repetition to dampen monotonic order effects.
- All algorithms sort an **identical copy** of each mode/seed input, verified elementwise against `std::sort`. Generation, cloning, and oracle computation/verification excluded from timing; each algorithm's internal allocations included.
- Each raw record contains the median/min/max milliseconds across three timed iterations; three per-mode input seeds. Group-level table times here are the *median of seed-level medians*. The `paired_summary.csv` records per-mode median of **three paired competitor/v14 time ratios**, which is preferable to dividing two marginal medians.
- K-prefix modes select from a table of K independently sampled full-width keys with replacement, so K is a *palette size*, not necessarily the exact count of distinct keys in an input.
- Workload labels `organpipe`, `sawtooth`, `alternating`, `rotated`, `local`, and `nearly` refer to their specific generators in `experiments/v14-full-gauntlet.cpp`.
- No data is merged across architectures or across algorithm runners for a given individual workload. **Important hardware nuance:** the three x86 group jobs ran on **different processors**, so even x86 results across groups should NOT be presented as a controlled identical-host comparison. The ARM group jobs reported the same Neoverse-N2 CPU model but separate virtualized runners.

## Hardware per dataset group

| Architecture | Job group | Observed CPU model | Raw rows |
|---|---|---|---:|
| x86-64 | entropy | Intel Xeon Platinum 8573C | 1,404 |
| x86-64 | cardinality | AMD EPYC 9V45 | 1,404 |
| x86-64 | ordered | AMD EPYC 9V74 | 1,053 |
| ARM64 | entropy | Arm Neoverse-N2 | 1,296 |
| ARM64 | cardinality | Arm Neoverse-N2 | 1,296 |
| ARM64 | ordered | Arm Neoverse-N2 | 972 |

All are virtualized GitHub Actions runners. Compiler reported `g++ 13.3.0`. Different x86 hardware is a **real confound for any all-modes aggregate rank**, even though every head-to-head comparison *within* one mode uses the same exact runner.

## Representative per-CPU findings — N=1,000,000, ms (lower is better)

### x86 (note group-specific CPU differences)

| Mode | RUNE v14 | VQSort | Best measured |
|---|---:|---:|---|
| Random full-width 64-bit [Intel Xeon] | 26.065 | **6.840** | VQSort |
| Sparse20 [Intel Xeon] | 14.758 | **6.813** | VQSort |
| Sparse16 [Intel Xeon] | 6.657 | see detailed CSV | RUNE v5 at 6.539; RUNE v14 within 2% |
| K1,024 [EPYC 9V45] | 2.491 | 1.638 | CAFS 1.208 |
| K8,192 [EPYC 9V45] | 2.652 | 2.858 | CAFS 1.808 |
| K65,536 [EPYC 9V45] | 8.263 | **3.455** | VQSort |
| K81,920 [EPYC 9V45] | 8.538 | **3.405** | VQSort |
| Sorted input [EPYC 9V74] | 1.021 | 11.385 | IPS4o 0.350 |
| Nearly sorted [EPYC 9V74] | 4.596 | 13.835 | RUNE v13 4.538; v14 within 2% |

**Strong counterevidence against generic RUNE superiority:** VQSort 6.840ms vs v14 26.065ms on the Xeon uniform64 1M case (~3.81× difference), and VQSort 3.455ms vs v14 8.263ms on AMD EPYC K65,536 (~2.39×).

### ARM64 Neoverse-N2

| Mode | RUNE v14 | VQSort | Best measured |
|---|---:|---:|---|
| Uniform64 | 15.696 | 17.310 | RUNE v13 13.631 |
| Sparse16 (widely spread mask) | 13.377 | 17.735 | standalone ARM sparse16 13.332; v14 within 2% |
| Sparse20 | **15.760** | 17.435 | RUNE v14 |
| K1,024 | 2.634 | 8.328 | RUNE v7 2.587; v14 within 2% |
| K65,536 | 10.707 | 18.163 | RUNE v13 10.519; v14 within 2% |
| K81,920 | 15.473 | 18.136 | RUNE v5 15.314; v14 within 2% |
| Sorted input | 0.901 | 0.351 | IPS4o 0.333 |
| Reversed input | 1.105 | see CSV | IPS4o 0.542 |
| Nearly sorted | **3.449** | see CSV | RUNE v14 (other RUNE variants within 2%) |

**ARM64 limitation:** On N=4M with 17 scattered varying bits, IPS4o won at ~59.5ms versus RUNE v14 ~73.1ms. V14's targeted 16- and 20-bit cases do not generalize to this workload and size.

## Complete result coverage and descriptive winner counts

Every CPU has **99 mode×size cases** (33×3), and all cases have all available implementation/seed combinations. Full tables are in:
- [x86 raw 3,861 rows](x86_raw.csv)
- [ARM64 raw 3,564 rows](arm64_raw.csv)
- [Full 2,475-row algorithm-by-workload summary with paired ratios](paired_summary.csv)
- [All 198 mode×size fastest-measured sorters](winners.csv).

For orientation only, counting whichever algorithm has the smallest observed *median seed-of-medians* even when a margin is tiny yields:
- **x86 (99 cases over three DIFFERENT CPU models):** VQSort 48, CAFS 16, five frozen/current RUNE versions combined 29, IPS4o 3, `std::stable_sort` 2, pdqsort 1.
- **ARM64 (99 cases over Neoverse-N2 virtual runners):** six RUNE variants combined 75, VQSort 10, IPS4o 9, `std::stable_sort` 3, spreadsort 1, pdqsort 1.

**CAUTION:** These are descriptive ranks for an intentionally varied benchmark suite, *not weighted market share*, confidence estimates, independent replications or fair globally comparable counts across different hardware. Near-equal RUNE variants in particular swap ranks due to ordinary noise. The `winners.csv` file marks all algorithms within a **2% descriptive margin** of the fastest (this is NOT a statistical confidence interval).

## Research interpretation and prioritized next work

1. **x86 high-entropy throughput is the biggest demonstrated architectural weakness.** RUNE suffers on broad random64 and medium/high-cardinality values, especially K81,920/131,072 at 4M. Investigate VQSort's vectorized partitioning and the candidate's frequency dispatch overhead without copying proprietary/competitor code. A mechanism-aware branch needs dedicated profiling and equivalent build layouts.
2. **ARM specialized spectrum is genuinely competitive within selected distributions.** Strong sparse16/20 and numerous K-palette wins remain. At 4M sparse17, VQSort/IPS4o are faster; existing v13/v14 dispatch heuristics remain workload-sensitive.
3. **Structured-input special cases need review.** IPS4o beats v14 on sorted/reversed inputs on ARM; even an algorithm with an excellent nearly-sorted path can lose badly on other structured families.
4. **Cross-group x86 CPU heterogeneity limits global summary counts.** Next replication should lock all groups onto a dedicated same-model pinned x86 host and a physical ARM64 host. Three seeds × three timed reps is an initial broad scan; repeat top surprises with more seeds and confidence bounds.
5. **Do not merge v14 based on these benchmark results alone.** Both v14 and this new benchmark PR are drafts. All baseline sources and RUNE implementation files are frozen.

**Evidence state:** complete **CI-verified broad comparator scan**, workload-specific empirical observations; **no universal SOTA claim**, no formal novelty/proof.
