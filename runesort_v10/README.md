# RUNE v10 — Guarded 10-bit LSD radix research candidate

**State: specified / exploratory. Not a release.** v7 remains frozen; v8 and v9 are NO-GO. This experiment changes only a high-entropy fallback branch after v7's exact frequency tiers. The existing proprietary LICENSE and main are unchanged.

## Mechanism and proof sketch
The existing 8-bit LSD sorter performs eight stable radix scatter passes on 64-bit integer keys. This candidate adds a 10-bit LSD worker with **seven stable passes**, six 10-bit digits plus a final four-bit digit (60+4=64). Each pass uses exact digit histograms and stable prefix-count scatter. Induction on passes proves the result is sorted by all 64 bits after the final pass. Each element is counted/scattered once per pass: O(7n+7·1024) time, O(n+7·1024) auxiliary memory in the word-RAM model; 56KiB of 64-bit histogram counters (versus 16KiB for byte radix), plus n×8 bytes of scratch. The sorter is not stable with respect to external records because its public input is values only.

Only for n>=524288, sampled descents>=20, at least 60 sample-varying bit positions, and 40<=duplicates_before_4K_key_table_rejection<=180, the new full-width worker is called. This gate makes no assumptions about the actual key domain; incorrect classification can change only runtime. All other paths are inherited directly from v7.

## Exploratory local evidence — not an end-to-end gain
Locally tested a standalone reference LSD implementation with widths 8,10,11,13,16 on an AMD EPYC 9V74 virtual machine, GCC 14.2, -O3 -march=native -DNDEBUG, n=1 million. On three seeds and four passes/repetitions, **10-bit radix improved some 82K-key samples** but worse on others, while 11-/13-/16-bit were not robust. High-bit bucketing followed by std::sort was substantially worse, and has been rejected. See research raw data in `experiments/radix-width-` artifacts on the branch.

Critically, this is an *engineering hypothesis*, not a novel radix algorithm. Fused LSD byte histograms, histogram cache tradeoffs, and wider-digit costs have extensive prior art:
- https://github.com/eloj/radix-sorting
- https://github.com/clausecker/radsort
- https://github.com/klauspost/radsort

## Decision gate
- Same-run v10 vs frozen v7 on ARM64 and x86-64, 5 fresh seed corpora, correctness oracle = std::sort; evaluate K~74K–200K, uniform64, random32, and sparse16.
- Require net speedup without severe per-workload regressions or significant extra memory use.
- If v10 loses end-to-end, record NO-GO and **do not merge**. If it survives, qualify against pinned Highway VQSort and CAFS with CPU-specific evidence. No SOTA claims from this experiment.
