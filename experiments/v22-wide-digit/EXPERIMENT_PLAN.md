# RUNE v22 — Wider digit LSD radix exploration

## Research contract
Derived directly from frozen RUNE v20 diagnostic tree `d172516a691d82914b432a8d22bbaef55d828f91`, NOT failed v21 code. No change to the production sorter, dispatch, main, license or historical branches.

**Claim to test:** x86-64 N=4M uniform64 8-pass radix scatter is slower than pinned Highway VQSort. Reducing stable scatters without changing output semantics could improve the gap. Compare two *conventional*, explicitly non-novel candidates:

- **radix11** — six base-2^11 stable LSD passes, one fused all-digit histogram scan; six 2048-bin uint32 histograms totaling 48KiB. Final 11-bit mask includes 2 unused high bits; sorting still exact.
- **radix16** — four base-2^16 stable LSD passes, each independently counts and prefixes a 65536-bin uint32 histogram (256KiB), reducing global scatter count 8→4 at cost of four count scans and larger counters footprint.

Both allocate one N-sized uint64 scratch without unnecessary initialization, have O(n) scratch and O(6n+6*2048) or O(8n+4*65536) work, and revert to the validated frozen v17 if n>UINT32_MAX for offset safety. Six or four even passes restore sorted array to original storage.

**Counterhypothesis:** larger histogram footprints, random counter updates and per-pass counting eliminate savings or regress on ARM. A result slower than v17 or pinned VQSort on the target is NO-GO. All sorted/alternating timings in these workers are *forced* and not equivalent to public dispatcher behavior.

**Pinned comparison:** same-host native x86 AMD EPYC / ARM Neoverse-N2, six algorithms (public v17, direct fixed8, radix11, radix16, pinned Google Highway VQSort SHA `c3407ccaaecffa735c911cae3971efa02263211d`, pinned IPS4o SHA `8d5a97bb4c4048a1eef18b537f191a31462b051a`), five predeclared seeds (20271021, 20271023, 20271027, 20271029, 20271031), 5 modes, 1M+4M arrays, one warmup + 3 randomized-order measured repetitions. Each candidate sort's entire output must match std::sort oracle. 600 measured row/array checks across both architectures. Small/boundary/large independently verified oracle tests; ASan/UBSan on both, portable x86 no BMI2; full existing CMake/CTest.

**Promotion gate:** repeatable meaningful (>5% relative to frozen worker) improvement on x86 N4M uniform64 and evidence of closing VQSort gap; no significant ARM regression; follow-up independent hardware replications/newest pinned sorters; no merge in this campaign. Retain raw rows for losses and status. No novelty, no publication-ready SOTA assertion.
