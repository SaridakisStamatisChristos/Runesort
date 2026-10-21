# v24: full public dispatcher with an x86-only prefetch route

Parent: v23 evidence commit `8d63bb31c2b1d33ef22a61289ba1bc3cf4431ce9`.
Question: does the v23 kernel result survive the full v17 dispatch decisions,
allocation, counting and output work? This is conventional radix/prefetch
engineering, with no novelty or SOTA claim.

## Frozen implementation and qualification protocol

Only on x86-64 and n >= 1,000,000, reuse v17's dispatcher and replace its existing
high-entropy route (>=20 sampled descents, >=60 sampled variable bits) with
v23 radix16, lookahead128. Do not change the later full-scan fixed8 route. All
earlier exact ordered/low-cardinality paths retain precedence. Smaller inputs,
ARM and `RUNE24_DISABLE` builds call frozen v17 directly. A text-level inheritance
check enforces the single worker replacement; test-only route counters verify
selection. Production builds contain no route counters.

The size cutoff and distance come from prior v23 x86 1M/4M results; they are not
optimized on v24 timings. A same-dispatcher distance-zero control isolates the
hint effect within this integration. Five algorithms: frozen RUNE17 public sort,
v24 full ablation0, v24 full prefetch128, pinned VQSort and sequential IPS4o.
No forced-worker timings are presented as public-sort timings.

Native x86-64 and ARM; n = 65,536, 999,999, 1,000,000 and 4,000,000. Ten modes:
uniform64, random32, K85000, sorted, alternating, reverse, sparse16,
almostsorted (n/1000 random swaps), sortedprefix (first64 ordered, rest random),
entropy_prefix (first64 random, rest increasing). The last two probe sample
misclassification while keeping exact output mandatory. Generators and mode
order are fixed in the benchmark source.

Qualification seeds: 20291001, 20291011, 20291021, 20291031, 20291041, with source
seed +229*mode_index and algorithm-order seed +447*mode_index. Local pilot uses
only seed907001 and uniform64. These qualification seeds are fresh relative to
v23 and the v24 pilot; shared hardware and synthetic generators limit independence.
One warmup and three measured trials per algorithm/seed/mode/size, randomized
algorithm order each round; all outputs compared exactly with an independent
std::sort oracle, with input copies and oracle outside timing. All raw trials
retained at round-trip precision. Expected 1,000 rows per ISA, 2,000 total,
6,000 measured sorts and 2,000 warmups. Pair within seed; aggregate medians of
trial medians and medians of same-seed latency ratios, not ratio of medians.

## Predeclared decision gates

1. Correctness: every exact comparison and route expectation, full CTest suite,
   ASan/UBSan, no-prefetch, dispatcher-disabled and x86 BMI2-off checks pass.
2. End-to-end benefit: on x86 uniform64 at BOTH 1M and 4M, candidate/frozen-v17
   median paired ratio <=0.95 and 5/5 seed wins.
3. Hint contribution: on x86 4M uniform64, candidate/full-ablation0 ratio <=0.95
   and 5/5 wins. Failure must not be hidden by the wider-digit gain.
4. Regression guard: candidate/frozen-v17 ratio <=1.05 in EVERY remaining
   mode-size-ISA cell, including ARM and below-threshold controls. These strict
   descriptive gates can fail from noise; report failures without post-hoc
   removal or automatic reruns until green.
5. Pinned-VQSort superiority requires <=0.95 and 5/5 wins at BOTH x86 target
   sizes. This is separate from integration eligibility and is not a SOTA test.

All ratios are latency ratios (lower is better). Gates are descriptive engineering
rules, not confidence intervals or significance tests. A single runner per ISA
cannot qualify all x86 microarchitectures. Even all gates passing would support
only conditional research promotion, with additional x86 CPUs needed before a
broad default. Preserve original license; no automatic merge or release.

## Correctness and resource argument

v17's unchanged shortcuts verify their own exact preconditions. The changed
sample route selects a universally correct radix worker, not a restricted-domain
algorithm, so a misleading prefix changes runtime only. v23's stable counting
passes and valid future-key prefetch address argument remain applicable. Its
uint32 histogram bound retains the fixed8 fallback above UINT32_MAX. This regime
is reasoned, not huge-allocation-tested. Public API remains ascending uint64 keys,
not stable records, O(n log n) worst-case time and O(n) auxiliary memory.

## Reproducibility and claim audit

Workflow pins VQSort/Highway `c3407ccaaecffa735c911cae3971efa02263211d` and IPS4o
`8d5a97bb4c4048a1eef18b537f191a31462b051a`, the historical revisions used in v23.
Record compiler, CPU, OS, source commit, raw CSVs and artifact digests. Recompute
summaries after download and audit full expected matrices, duplication, positive
finite trials and consistency of median/min/max. Archive a claim-level verdict
distinguishing exact output, measured latency, causal attribution, regression
preservation, ISA generalization and comparator superiority. Without PMU data,
do not infer cache-miss/RFO reductions from latency alone.
