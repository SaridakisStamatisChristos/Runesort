# RUNE v14 — ARM64 16–23-bit spectrum integration

**Research decision: CONDITIONAL GO for further investigation, HOLD any merge or release.** This research PR is stacked on frozen v13 PR #12. `main`, proprietary LICENSE, and the original v7/v13/ARM-sparse16 research sources remain unchanged.

## What changed

V13's ARM64-only 17–23-bit topology-aware radix dispatch is unchanged. V14 adds a narrow ARM64-only branch after the **full-array** variability and range scan:

```cpp
#if defined(__aarch64__)
  if(n>=524288 && varied==16 &&
     detail::fifth_distinct_reconstruct(a,n)) return;
#endif
```

This reuses the already implemented exact bounded-frequency reconstruction from the independent sparse16 research [PR #5](https://github.com/SaridakisStamatisChristos/Runesort/pull/5). The full-array mask certifies at most 2^16=65,536 distinct keys; the reused frequency table permits 81,920. Equality comparisons resolve collisions, a 32-probe cap detects pathological collisions, and rejected scans leave the input unchanged for the original exact fallback. No assumptions depend on just the first 64 sampled elements.

**Complexity (fixed 64-bit unsigned keys):** bounded table scans O(n) with at most 32 probes each, then O(K log K) comparator sorting of distinct keys with K<=65,536, O(n) reconstruction; hash table and K-key arrays have fixed domain-bounded space. The general fallback retains v13's original complexity. This is an implementation-level synthesis, not a novel asymptotic sorting result.

## Refutation and repair

The first v14 attempt added `packed_digits_need_extract(variable)` to the sparse16 condition. It was unjustified: v13's **compact-domain counting** calls portable `extract()` even when all 16 varying bits are contiguous (the packed-digit predicate describes a *different* sorting path). The original guard was correctly tested but slower for contiguous16: at 4M, ARM64 initial v14 ~69.3 ms versus original PR#5 ~38–42 ms.

The guard was removed in source commit **`5ae0f9fc8b45edd6113d05b14b4706bd43565498`**, preserving the exact failure fallback. Its original evidence is archived separately in [superseded-guard raw CSV](superseded_guard_arm64_raw.csv) and [summary](superseded_guard_arm64_summary.csv). **The old benchmark must not be mistaken for a benchmark of final v14.**

## Final-source native qualification

- [Final native dual-CPU CMake, four-way spectrum, ASan/UBSan and no-BMI2 workflow](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39433665848).
- ARM64 **Neoverse-N2**; x86-64 GitHub virtualized runner; GCC 13.3 C++17, `-O3 -march=native -DNDEBUG` for native benchmark.
- Four versions compared directly: corrected RUNE v14, frozen v13, frozen PR#5 ARM16 candidate, and frozen v7. N=524,288, 1M, 4M; 13 distributions including seven 16-bit bit-mask shapes, sparse17/20/23, and high-entropy/cardinality controls.
- Five new independent input seeds, randomized sorter call order, four timed repetitions/seed after warmup. The same full unsigned-64-bit input array was used for every algorithm, and output was checked against independent C++ `std::sort`. Sorting allocations included; input copy and oracle excluded from the timed interval.
- **1560/1560 native benchmark rows matched the oracle**, 780 per CPU; both native benchmark jobs passed. Source identity is the corrected hash above.

### ARM64 native medians, milliseconds (smaller is better)

| Mask / n | v13 | v14 | Frozen ARM16 PR#5 |
|---|---:|---:|---:|
| distributed16 / 1M | 18.331 | **13.627** | 13.692 |
| contiguous16-mid / 1M | 18.974 | **12.476** | 12.464 |
| split16 / 1M | 18.756 | **12.374** | 12.359 |
| contiguous16-mid / 4M | 68.758 | **41.081** | 40.926 |
| split16 / 4M | 69.092 | **39.652** | 39.672 |
| four separated four-bit groups / 4M | 69.956 | 39.882 | **39.814** |
| distributed sparse20 / 4M | 72.381 | **72.135** | 156.588 |
| distributed sparse23 / 4M | 71.169 | 71.178 | 188.527 |

V14 combines the previously separate classes of benefits on the tested ARM runner. Comparisons of source variants at the percent level are susceptible to virtual-machine variance; differences smaller than a few percent are not reliable directional claims.

## Final-source pinned external qualification

- [Independent ARM64 and x86 external-runner workflow](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39433665855): **two of two jobs passed**.
- Comparison: corrected v14, v13, frozen ARM16, pinned Google Highway VQSort (commit `c3407ccaaecffa735c911cae3971efa02263211d`) and pinned sequential IPS4o (`8d5a97bb4c4048a1eef18b537f191a31462b051a`). C++20 and GCC13.3, native architecture optimizations. Competitor sources were fetched at run time and remain under their original licenses.
- Independent five held-out seeds; N=1M/4M; nine workload families; 4 measured repetitions + one warmup; randomized order, same input and exact `std::sort` oracle.
- **900/900 corrected-source external benchmark rows matched the oracle**, 450 per CPU.

### ARM64 external comparison, n=1M (median ms)

| Workload | v14 | VQSort | IPS4o |
|---|---:|---:|---:|
| distributed16 | **13.279** | 17.742 | 15.566 |
| random16 | **13.146** | 17.961 | 15.548 |
| contiguous16-mid | **12.244** | 18.337 | 15.510 |
| split16 | **12.287** | 17.507 | 15.522 |
| sparse20 | **15.851** | 17.631 | 18.890 |
| sparse23 | **16.005** | 17.547 | 18.906 |

At n=4M the corrected v14 was likewise faster than these pinned two external comparators on these selected distributions. This is a **real, bounded ARM64 workload-specific superiority observation**, not a statement of global SOTA.

## Adversarial verification and remaining gates

- **Original full ARM64 sanitizer finished PASS** at 2026-10-20T14:16:24Z: 828 canonical adversarial full-array cases plus exact-frequency rejection-without-input-mutation; no ASan or UBSan finding. [Original full four-job run](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39433665848) is **4/4 GREEN**: ARM64 and x86 native CTest/benchmark, ARM64 and x86 ASan/UBSan, x86 portable `-mno-bmi2`.
- The ARM64 sanitized full-matrix run was considerably slower than x86 under GitHub virtualized runners. A separate [six-pattern-per-architecture sharded sanitizer workflow](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39437462615) was added to improve future CI practicality **without changing the algorithm source or removing any mask/size/width pattern**. The original comprehensive test stays in CTest; sharded generated tests derive from that exact source and fail closed if its layout changes. Treat sharded outcomes independently from the now-passing original job.
- **Independent x86 preprocessor and object-code identity checks passed**: [GCC 13.3 + Clang 18.1, native x86 + portable no-BMI2 (all four passes)](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39437867004). For the same sorting entry point built sequentially from identical source paths and flags, v13 and v14 produced **identical preprocessed C++ and byte-identical object files**. This substantially narrows the x86 performance discrepancy: no x86 codegen change was observed under these profiles. The earlier native/external x86 timing differences of ~2–8% warrant dedicated-host repetition and scheduler/CPU-affinity control, rather than assigning causality to an ARM64-only source block.
- Corrected ARM64 external v14 sometimes trails archived ARM16 by ~3–5% despite invoking the same sparse16 helper and gate; repeat on a physical CPU before claiming one implementation wins on every sparse16 class.
- Cache occupancy, frequency-tier probe behavior, allocator costs, 16-bit cardinality distributions, and different ARM microarchitectures remain underexplored.
- The benchmark does not address universal speed against VQSort on x86 high-entropy 64-bit domains or all possible input distributions.
- No novelty claim: radix sorting and bounded hash-frequency reconstruction have existing prior art. This research concerns workload- and ISA-aware dispatch of existing exact mechanisms.

## Reproducibility

- [Corrected ARM64 native raw](native_arm64_neoverse_n2_raw.csv) · [native summary](native_arm64_neoverse_n2_summary.csv)
- [Corrected x86 native raw](native_x86_raw.csv) · [native summary](native_x86_summary.csv)
- [External ARM64 raw](external_arm64_neoverse_n2_raw.csv) · [external summary](external_arm64_neoverse_n2_summary.csv)
- [External x86 raw](external_x86_raw.csv) · [external summary](external_x86_summary.csv)
- [Initial rejected guard raw](superseded_guard_arm64_raw.csv) · [its summary](superseded_guard_arm64_summary.csv)

For all summaries, time values are medians of seed-level medians; `ratio_<competitor>` is the median of paired per-seed competitor/v14 runtime ratios, not necessarily the ratio of two displayed median times. Treat CPU evidence as non-interchangeable.

**Evidence ledger:** Corrected-source sorting outputs empirically validated within the stated workloads; full-range correctness analyzed via exact reconstruction/fallback invariants; ARM64 targeted speedup empirically supported by virtualized same-run competitors; cross-platform/universal SOTA unproven. **HOLD MERGE.**
