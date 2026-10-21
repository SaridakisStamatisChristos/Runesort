# RUNE v23 — destination-prefetch qualification

**Verdict: conditional GO for x86-only prefetch128 mechanism follow-up; NO-GO for unconditional integration or VQSort superiority. Keep this research PR unmerged.**

Benchmarked source: `6566ed06c6258cb34534d61564df1f0ffd6b8b07`, based on frozen v22 `49dddfa54ce9ef6024d7796f7c9577db899b8f59`. Existing public RUNE headers, dispatcher, license and main are unchanged. The final evidence archive also corrects a workflow step label and discloses pilot seed reuse; it changes no measured C++ source.

## Verification and reproducibility

- [Qualification CI](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39555422531): four jobs passed (native x86, native ARM, both ASan/UBSan); x86 BMI2-off and both hint-disabled fallback builds passed.
- [Push CI](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39555422432) and [PR CI](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39555455981) passed: **six jobs green for the benchmarked commit**. Local CTest: 28/28 passed.
- **900 complete observations**: 9 algorithms × 2 sizes × 5 modes × 5 seeds × 2 ISAs. Each contains 3 measured trials plus one warmup: **2,700 measured sorts and 900 warmups**, every full output checked against an independent std::sort oracle. These are 900 observations of repeated trials, not 2,700 independent inputs.
- Differential tests: 388 arrays plus 30 guarded subarrays per test invocation; empty nullptr, lookahead/tail boundaries, all-equal, alternating extremes, ordered/reverse, sparse high bits and million-scale inputs. Tests are not a formal proof.
- Both downloaded artifact matrices were independently revalidated locally; recomputed paired summaries match their CI artifacts byte for byte. CI artifact identities/digests and exact run state are in [CI_RESULTS.json](CI_RESULTS.json). File hashes and fixed gate results are in [EVIDENCE_LEDGER.json](EVIDENCE_LEDGER.json).
- GCC 13.3, `-std=c++20 -O3 -march=native -DNDEBUG -pthread`; x86 AMD EPYC 7763 VM and ARM Neoverse-N2 VM. Detailed untouched provenance: [x86](x86_64_provenance.txt), [ARM](arm64_provenance.txt). These are not controlled comparisons of absolute latency between CPUs.
- Historical comparator pins: Highway `c3407ccaaecffa735c911cae3971efa02263211d`, sequential IPS4o `8d5a97bb4c4048a1eef18b537f191a31462b051a`. No latest-version or universal SOTA claim.
- Copy/generation/oracle work is outside timing; algorithm order is randomized each round. All individual trial times are retained at double round-trip precision. Five fixed new CI seeds were used; the first was also used in an exploratory local pilot. This is **not a wholly held-out test set**. Gates were fixed before CI timings; see [PLAN.md](PLAN.md).

## Primary N=4M uniform64 target (milliseconds, smaller is better)

| ISA | Frozen fixed8 | Frozen v22 radix16 | Ablation0 | Prefetch32 | Prefetch128 | Pinned VQSort |
|---|---:|---:|---:|---:|---:|---:|
| x86_64 | 123.165 | 100.114 | 101.454 | 95.735 | 91.310 | 54.612 |
| arm64 | 74.190 | 90.072 | 90.113 | 126.813 | 126.965 | 75.463 |

- **x86 prefetch128 passes the prespecified mechanism gate:** median same-seed candidate/v22 ratio **0.912×** (8.8% lower latency); candidate/ablation0 **0.900×**, with **5/5 wins against both**. Candidate/fixed8 **0.742×**. This is a worker-level improvement, not a measured integrated public-dispatcher improvement.
- **x86 VQSort closure still fails:** prefetch128/VQSort **1.669×**, losing all five seeds. The remaining latency gap is about 67%, not evidence of superiority.
- Prefetch32 wins all five x86 seeds against frozen v22, but ratio **0.956×** misses the prespecified <=0.95 gate. It is not promoted by relaxing the threshold.
- **ARM rejects unconditional replacement:** prefetch128/v22 **1.407×** (~40.7% slower), losing all five seeds. Five of six ARM nonordered size/mode cells exceed the 5% regression limit for each candidate.

All displayed milliseconds are medians of five seed-level medians. Ratios are separately medians of seed-matched ratios; they need not equal quotients of displayed medians. Five-seed win counts are descriptive, not statistical significance estimates.

## Additional 4M modes, prefetch128 only

| ISA | Mode | Candidate ms | Ratio to v22 | Ratio to VQSort |
|---|---|---:|---:|---:|
| x86_64 | uniform64 | 91.310 | 0.912× | 1.669× |
| x86_64 | random32 | 69.140 | 0.989× | 1.269× |
| x86_64 | K85000 | 84.022 | 0.862× | 1.516× |
| x86_64 | sorted | 91.248 | 0.902× | 1.807× |
| x86_64 | alternating | 54.018 | 1.140× | 17.512× |
| arm64 | uniform64 | 126.965 | 1.407× | 1.677× |
| arm64 | random32 | 95.402 | 1.141× | 1.265× |
| arm64 | K85000 | 82.838 | 1.114× | 1.336× |
| arm64 | sorted | 126.390 | 1.402× | 79.634× |
| arm64 | alternating | 103.354 | 1.602× | 30.464× |

Full 1M/4M paired summaries: [x86](x86_64_paired_summary.csv), [ARM](arm64_paired_summary.csv). Raw trial CSVs: [x86 1M](x86_64_1m.csv), [x86 4M](x86_64_4m.csv), [ARM 1M](arm64_1m.csv), [ARM 4M](arm64_4m.csv). Sorted/alternating candidate timings force the worker and are negative controls, not public RUNE end-to-end performance.

## Interpretation and next bounded work

The x86 ablation supports a useful implementation effect from lookahead in this configuration. It does not establish fewer cache misses, reduced RFO, or a hardware-level causal explanation; no PMU counters were collected. The same mechanism is harmful on the ARM runner. These are conventional LSD radix and software prefetch techniques, not a novel sorting algorithm.

The next defensible experiment is an isolated x86/size-gated dispatcher candidate using prefetch128, benchmarked end-to-end against the frozen full sorter, raw workers and pinned VQSort, on fresh seeds and additional CPUs. Keep small arrays, narrow domains, ordered/low-cardinality inputs and ARM on their independently qualified paths. It must include scout/gating overhead and demonstrate no material regressions before integration. Do not merge this research worker simply because its x86 kernel gate passed.

Execution used native C++ tools and the OmO Controller native fallback. The real OmO runtime probe reported no installed binary; no real OmO worker execution is claimed.
