# RUNE v21 — MSB bucket-local sorting, cross-architecture qualification

**VERDICT: CONDITIONAL RESEARCH GO for high-entropy uniform64 bucket_radix7 only; NO-GO as unconditional public RUNE replacement, NO-GO for bucket_stdsort. HOLD MERGE.**

The tested code is commit `e28641acad4095460c38062e7aa06f6b335b1e07`. It is a separate experimental kernel; none of the production RUNE dispatcher, underlying v17 source or main branch has changed.

## Verified evidence

- [Native x86 + ARM exact external benchmarks and sanitizers](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39521017146): all **4/4 jobs green**, comprising two full repository CMake/CTest + native benchmark jobs, and two ASan/UBSan differential test jobs including portable no-BMI2 on x86.
- [Standard push CI](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39521017115) and [standard PR CI](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39521062938): **2/2 passed**. Total 6/6 CI jobs.
- **700/700** measured complete-array outputs correct versus independently computed `std::sort` oracle: 2 ISAs × 2 sizes × 5 modes × 5 seeds × 7 sorters. Every paired seed has all seven observations. Native and sanitized differential suites passed.
- 1 warmup + 3 measured randomized-order sorts per algorithm/input, GCC 13.3 C++20 `-O3 -march=native -DNDEBUG -pthread` for full external comparison, same input, generation/copy/verification outside timed region.
- Historical frozen [Highway VQSort](https://github.com/google/highway/commit/c3407ccaaecffa735c911cae3971efa02263211d) and [IPS4o](https://github.com/SaschaWitt/ips4o/commit/8d5a97bb4c4048a1eef18b537f191a31462b051a) commits; not necessarily latest upstream.

## Native 4M same-machine timings (milliseconds, lower is better)

| CPU | Distribution | Frozen RUNE17 public | v21 local radix | v21 bucket stdsort | Pinned VQSort | Paired v17/v21 speedup (wins) |
|---|---|---:|---:|---:|---:|---:|
| x86_64 | uniform64 | 52.255 | 51.188 | 208.253 | 60.304 | 1.020× (4/5) |
| x86_64 | random32 | 43.339 | 45.492 | 310.117 | 60.335 | 0.952× (0/5) |
| x86_64 | K85000 | 58.378 | 55.879 | 162.198 | 62.002 | 1.044× (5/5) |
| x86_64 | sorted | 4.094 | 49.399 | 39.982 | 50.005 | 0.083× (0/5) |
| x86_64 | alternating | 2.828 | 44.314 | 66.497 | 2.651 | 0.064× (0/5) |
| aarch64 | uniform64 | 62.154 | 54.237 | 132.636 | 76.145 | 1.152× (5/5) |
| aarch64 | random32 | 46.261 | 78.366 | 202.321 | 76.261 | 0.589× (0/5) |
| aarch64 | K85000 | 66.578 | 64.591 | 103.488 | 61.907 | 1.031× (5/5) |
| aarch64 | sorted | 3.596 | 61.164 | 46.170 | 1.455 | 0.059× (0/5) |
| aarch64 | alternating | 2.312 | 82.490 | 50.391 | 3.193 | 0.028× (0/5) |

Times are medians of seed medians. Ratios are **medians of matched seed baseline/candidate ratios** and may differ from the displayed time ratios. Within same-run paired comparisons only; no cross-ISA or cross-run speedups.

## What worked, and what failed

- **ARM Neoverse-N2 4M uniform64:** baseline **62.154 ms**, v21 bucket radix **54.237 ms**, pinned VQSort **76.145 ms**; candidate wins **5/5 seeds** and has ~15% median *speedup ratio* over frozen RUNE17 on this runner. Initial evidence supports the cache-locality mechanism.
- **x86 EPYC 9V74 4M uniform64:** baseline **52.255 ms**, v21 bucket radix **51.188 ms**, pinned VQSort **60.304 ms**. Candidate wins **4/5 seeds** but gain is only ~2%; insufficient for a claim of durable x86 advantage.
- **ARM 4M random32:** RUNE17 **46.261 ms** vs bucket radix **78.366 ms**, a large **regression**. All numbers fit in one MSB bucket, defeating cache partitioning. x86 random32 baseline **43.339 ms** vs candidate **45.492 ms** also regresses. This forbids unconditional integration.
- **Sorted and alternating:** the candidate always performs partitioning and per-bucket sorting; full RUNE17 exploits fast certified exits. Huge observed regressions mean v21 must **never** replace the public RUNE sorter without safe gated routing.
- **bucket_stdsort** is dramatically slower on high-entropy and lower-width input, so it is rejected as a general candidate.
- **K85000** bucket-local radix is moderately better than v17 on both measured platforms, but not necessarily better than external VQSort/IPS4o, and must not be cherry-picked for generic superiority.

### Cross-run hardware contradiction

**The x86 CPU in v21 is AMD EPYC 9V74, NOT the AMD EPYC 7763 used for v20. This hardware difference changes the relative VQSort/RUNE outcome. Cross-run ratio improvements are invalid; only same-run paired v21 ratios are evidence.** Both have the x86 label, but v20 x86 RUNE17 4M uniform64 105.624ms vs VQSort 54.996ms on EPYC 7763, whereas v21 shows RUNE17 52.255ms vs VQSort 60.304ms on EPYC 9V74. This reversal is strong evidence that hardware-specific conclusions cannot be generalized. Both within-run measurements may be correct without contradiction.

## Architecture/complexity/correctness

MSD 256-way high-byte partition uses O(n) scratch. Local 7-pass stable LSD radix on each MSB bucket followed by tiny-bucket std::sort gives O(8n + 256*7*256 + Σ_{tiny}m log m) worst-case time and O(n) additional memory. The all-one-bucket worst case still has 7 full-array local passes and can regress severely. This is a conventional multi-stage MSD+LSD radix hybrid, not claimed novel. American flag sorting is prior art but v21 is **not** its in-place variant. The complete-array `std::sort` oracle, boundary cases, extreme uint64 values, duplicates and sanitizer checks empirically validate sorting, not prove code free of all possible defects.

## Evidence files

- [x86 350 raw observations](x86_raw.csv) (EPYC 9V74)
- [ARM64 350 raw observations](arm64_raw.csv) (Neoverse-N2)
- [20 grouped paired summaries](paired_summary.csv)
- [provenance.json](provenance.json)

## Decision

Leave PR #21 open as a **draft research archive**. The v21 idea is useful but **not mergeable as a general sorter**: future work would require inexpensive, independently certified high-byte-distribution/entropy gating that preserves the current public RUNE17 behavior on low-entropy and sorted inputs; then 2+ controlled repeats on a pinned x86 target (including EPYC 7763 or equivalent), rerun against newer external revisions, and check false positives/adversarial distributions. Do not alter the acceptance threshold after seeing these results. Avoid reviving failed v18 buffer staging or v19 per-digit histograms.
