# RUNE v20 — Independent pinned VQSort/IPS4o full-width diagnostic

**Verdict: INVESTIGATE SCATTER DESIGN; NO production kernel change, NO global SOTA claim, HOLD MERGE.**

Qualified source commit: `4ba2b0c338c794584f78d0f868e9bf5bee0d6b56`. Frozen v17 from `927d29b778b75f67e8a908cffa914a5f6e88929f`. Four qualification jobs [all green](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39519319443), including two native external comparator runs and two ASan/UBSan checks. Standard [push CI](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39519319385) and [PR CI](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39519365351) passed: **6/6 overall**.

## Reproducibility / ground truth

- **500/500** sort output observations exactly match a full independent `std::sort` oracle (250 x86 and 250 ARM).
- **336/336** phase observations from instrumented, independently oracle-verified radix arrays (168 x86 and 168 ARM).
- Five fixed independent seeds, five workload modes, n=1M/4M, one warmup plus three measured sorts in randomized method order per input; copy/generation/oracle outside timed sorts.
- `g++ -O3 -march=native -std=c++20 -DNDEBUG -pthread`; GitHub-hosted Ubuntu 24.04 virtual x86/ARM runners. Physical CPU and compiler details in [GitHub Actions run](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39519319443).
- Exact upstream comparator revisions: [Highway VQSort `c3407ccaaecffa735c911cae3971efa02263211d`](https://github.com/google/highway/commit/c3407ccaaecffa735c911cae3971efa02263211d), [IPS4o `8d5a97bb4c4048a1eef18b537f191a31462b051a`](https://github.com/SaschaWitt/ips4o/commit/8d5a97bb4c4048a1eef18b537f191a31462b051a). They are reproducible historical versions, **not claimed to be newest**. No external source vendored or modified.
- All sorters compare **unsigned 64-bit ascending keys**; `IPS4o_seq` is the ordinary sequential `ips4o::sort`, not parallel mode. `std::sort` listed as comparator and independent correctness oracle.

## Native full comparator experiment, N=4M (milliseconds, lower is better)

| CPU | Input | Full RUNE17 | Raw 8-pass RUNE17 | VQSort | IPS4o | RUNE / VQ paired ratio | RUNE wins |
|---|---|---:|---:|---:|---:|---:|---:|
| x86_64 | uniform64 | 105.624 | 105.552 | 54.996 | 107.469 | 1.914× | 0/5 |
| x86_64 | random32 | 69.058 | 72.023 | 55.337 | 107.451 | 1.248× | 0/5 |
| x86_64 | K85000 | 83.944 | 82.724 | 56.376 | 64.867 | 1.492× | 0/5 |
| x86_64 | sorted | 3.805 | 107.237 | 48.633 | 2.577 | 0.078× | 5/5 |
| x86_64 | alternating | 3.470 | 47.543 | 2.473 | 10.597 | 1.393× | 0/5 |
| arm64 | uniform64 | 73.304 | 74.806 | 76.305 | 81.857 | 0.960× | 3/5 |
| arm64 | random32 | 52.444 | 86.605 | 76.286 | 81.687 | 0.690× | 5/5 |
| arm64 | K85000 | 67.939 | 67.484 | 61.752 | 53.877 | 1.100× | 0/5 |
| arm64 | sorted | 3.677 | 68.838 | 1.461 | 1.456 | 2.497× | 0/5 |
| arm64 | alternating | 2.453 | 97.063 | 3.365 | 9.470 | 0.723× | 5/5 |

Displayed milliseconds are medians of five seed-level medians; ratio columns are **median of five seed-matched RUNE/VQSort timing ratios**, not the ratio of displayed medians. They quantify relative latency, so values greater than one mean RUNE is slower. x86 and ARM absolute times must not be compared as a controlled A/B.

## Mechanism diagnosis

- **x86 full-width 4M uniform64:** full RUNE17 **105.62 ms** vs direct eight-pass worker **105.55 ms**; pinned VQSort **55.00 ms**. There is about a **1.91×** full-RUNE/VQSort paired latency ratio. On this workload, sample selection, duplicate probes and dispatch are not the dominant x86 gap because the raw worker already takes virtually identical time.
- **ARM full-width 4M uniform64:** full RUNE **73.30 ms** vs worker **74.81 ms**, VQSort **76.31 ms**. RUNE is competitive with the pinned VQSort on this specific ARM runner; not an architecture-wide or latest-version SOTA claim.
- **x86 instrumented 4M uniform64:** approximate `90.3%` of measured whole-kernel time is in eight stable scatter passes. ARM corresponding fraction `81.9%`. Both are clock-instrumented, not PMC/perf hardware attribution.
- **x86 random32:** full RUNE vs worker are similar (69.06 vs 72.02 ms at 4M), but dispatcher chooses faster paths on ARM (52.44 vs 86.60 ms). This disproves any broad claim that dispatcher always hurts or always helps.
- **K85000, sorted, alternating are controls**; sorted/alternating forcibly executing the eight-pass worker does not represent the public sorter, whose early exits can have dramatically different timings.
- The evidence supports **prioritizing scatter throughput on x86**, but does not establish memory cache misses or any particular new design. Previous [v18 cache-staged buffered radix](https://github.com/SaridakisStamatisChristos/Runesort/pull/18) badly failed; do NOT revive unchanged buffering. [v19 compact fused histogram](https://github.com/SaridakisStamatisChristos/Runesort/pull/19) showed only ~0.3% x86 gain; no production merge.

## Evidence
- [x86 raw external comparisons](x86_64_comparison_raw.csv), [ARM raw comparisons](arm64_comparison_raw.csv)
- [x86 instrumented phases](x86_64_phase_raw.csv), [ARM instrumented phases](arm64_phase_raw.csv)
- [paired_comparison_summary.csv](paired_comparison_summary.csv) and [phase_summary.csv](phase_summary.csv)
- [provenance.json](provenance.json)

## Decision

Keep PR as **unmerged research infrastructure and evidence**. For v21, choose an alternative that reduces random scatter cost without the failed v18 per-bucket bookkeeping, and first falsify it on identical x86 full-width cases with independent arrays/oracle. A good next discriminating step is physical hardware counters for store/RFO and cache behavior when permitted, plus current-version external comparisons. Do not assume v20 itself changes sorting performance.
