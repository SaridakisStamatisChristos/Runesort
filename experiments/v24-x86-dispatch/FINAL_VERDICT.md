# v24 result: full-dispatch gain on Intel, integration HOLD

Measured source: `54ac7bf6e30224712e9008cd46ff173fff30974f`.
Parent: v23 archive `8d63bb31c2b1d33ef22a61289ba1bc3cf4431ce9`.
[Draft PR #26](https://github.com/SaridakisStamatisChristos/Runesort/pull/26).
[Qualification run](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39559531907).

**Do not promote the broad x86 policy.** Full-dispatch benefit and hint-contribution
gates pass on the measured Intel Xeon Platinum 8573C. The regression guard fails
in two x86 cells; a separate local AMD EPYC 9V74 pilot is substantially slower
than v17. The pinned VQSort superiority gate also fails. ARM fallback passes all
40 guard cells. All six CI jobs pass correctness/build verification; green CI
does not mean the performance gates passed.

## Measured end-to-end results

Times below are medians across five seed-level trial medians, in milliseconds.
Ratios are medians of same-seed ratios, so they need not equal ratios of the
displayed medians. All dispatch decisions, allocations and sort work are timed.

| CPU / uniform64 | N | Frozen v17 | Full ablation0 | Full prefetch128 | Pinned VQSort | Candidate/v17 | Wins |
|---|---:|---:|---:|---:|---:|---:|---:|
| Intel Xeon Platinum 8573C | 1M | 25.672 | 45.097 | 18.573 | 6.668 | 0.723461 | 5/5 |
| Intel Xeon Platinum 8573C | 4M | 215.954 | 216.681 | 85.705 | 30.243 | 0.392491 | 5/5 |
| ARM Neoverse-N2 | 1M | 14.691 | 14.767 | 14.665 | 17.364 | 1.001594 | 2/5 |
| ARM Neoverse-N2 | 4M | 62.108 | 62.243 | 61.970 | 75.643 | 0.997782 | 3/5 |

The Intel candidate has 27.7%/60.8% lower paired latency than v17 at 1M/4M.
At 4M its paired ratio to the same-dispatcher distance-zero ablation is
0.399216, with 5/5 wins. It still takes 2.786723x/2.838843x pinned VQSort's time
at the two target sizes, with 0/5 wins. This is a CPU-scoped engineering result.

The v23 x86 runner was AMD EPYC 7763; v24's runner is Intel. Differences between
the two experiment versions' absolute numbers cannot be attributed to code
alone. Comparisons within each v24 seed use the same source array and machine.

## Failures retained without rerunning until green

| Intel regression cell | Candidate ms | v17 ms | Paired ratio | Candidate wins |
|---|---:|---:|---:|---:|
| random32, 1M | 18.799 | 17.984 | 1.055304 | 1/5 |
| sortedprefix, 4M | 261.351 | 231.385 | 1.073193 | 1/5 |

These fail the predeclared <=1.05 limit. Neither distribution selects the changed
worker: random32 lacks 60 variable sample bits; sortedprefix has no sampled
descent. Identical inherited decisions therefore do not establish identical
latency. Code placement, compiler decisions and measurement variability remain
possible explanations; no causal diagnosis is established. All 40 ARM cells
pass; its largest paired ratio is 1.033897 (entropy_prefix, 4M). Overall, 2 of 78
regression-guard cells fail. See `GATES.json` for every check, including passes.

The separate local 4M uniform64 pilot (seed907001, AMD EPYC 9V74 VM) measured
v17 69.604 ms, ablation0 119.559 ms, candidate 100.746 ms: candidate/v17
1.447427, about 44.7% slower. It ran after local builds/tests completed. This
single-seed counterexample is not a full qualification or a CPU-family diagnosis,
but it blocks an all-x86 recommendation. Local external-comparator slots are
explicitly labeled std::sort controls, not VQSort/IPS4o. The earlier contended
1M pilot is also archived; it is excluded from latency conclusions.

## Correctness, provenance and research audit

- Both native runs passed 29/29 CTest tests. Differential tests cover 134 exact
  arrays, four guarded subarrays, nullptr/zero, threshold edges, narrow domains,
  misleading samples and route expectations. Five selected routes on x86; zero
  on ARM or dispatcher-disabled builds.
- Both ASan/UBSan jobs passed with leak detection enabled. Hint-disabled and
  dispatcher-disabled builds passed on both ISAs; x86 BMI2-off passed.
- Exactly 1,000 aggregate rows per ISA: 2,000 total, 6,000 measured sorts and
  2,000 warmups, each checked against a full std::sort oracle. Local pilot rows
  are additional and are not included in these qualification counts.
- Five new qualification seeds are separate from the pilot. Three measured
  trials per cell are retained, not only best times. Algorithm order is shuffled
  each round; input copy and oracle construction are outside the timer.
- Both downloaded ZIP digests match GitHub metadata. Both summaries reproduce
  byte-for-byte from raw CSVs. A separate direct calculation also verified all
  160 summary rows, all paired ratios and all seed-win counts.
- The inherited dispatcher differs at only one worker call, verified before
  benchmarking. Frozen v17/v22/v23 headers remain unchanged. Source/artifact
  hashes and the claim-level audit are in `EVIDENCE_LEDGER.json`; job and artifact
  identities are in `CI_RESULTS.json`.

Reproducibility level: executed and reproduced within this audit workflow,
with disclosed cross-machine disagreement. Not robustly reproduced universal
performance. The evidence archive commit adds reports/data only; the source
commit above is what all CI jobs measured. No benchmark threshold was adjusted
after observing v24 measurements.

Limits: virtualized runners, one qualification session per ISA, five synthetic
seeds, no confidence intervals or significance claim, no PMU-based causal
attribution, and no allocation test above UINT32_MAX. Comparators are the pinned
historical upstream revisions in `PLAN.md`, not a current SOTA survey. No novelty
claim or research-paper readiness claim is made.

## Next development boundary

Retain v24 as a research candidate, without merging or enabling it in the default
sorter. Investigate the inactive-path regressions and qualify a narrower opt-in
or CPU-specific policy on fresh data, including the AMD counterexample. Simply
changing the architecture check to all x86 is already disproved as a safe
performance recommendation. Preserve the successful ARM fallback.

To reproduce the archived analysis, run `summarize.py` for each ISA using its
four raw size CSVs, then run `evaluate.py --output GATES.json` with the two summary
CSVs. The workflow records complete build/benchmark commands and dependency pins.
