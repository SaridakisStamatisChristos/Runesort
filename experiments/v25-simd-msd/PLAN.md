# v25: SIMD MSD partitioning and register-network leaves

Parent: `4399eeb29802875b609efac0c29e6eac73702cda` (v24 archive).
Contract: sort unsigned 64-bit keys in ascending order, single thread, exact
multiset, no record-stability promise. Minimize complete public-sort latency;
retain O(n) auxiliary storage and a bounded worst-case algorithm. No novel or
universal SOTA claim is inferred from passing correctness checks.

## Reconstruction and candidate selection

v20 identified scalar scatter as the main full-width kernel cost. v18 buffering
failed; v19 histogram compaction barely helped. The two v21 branches differ:
bucket+std::sort failed, while high-byte partition+local radix7 showed a small
x86 benefit and a useful ARM result, but failed as an unconditional sorter.
v22/v23 wide-digit and prefetch workers improved some CPUs; v24 full-dispatch
integration failed inactive-route guards and a local AMD pilot.

Two materially different paths were screened on local seed 980003:

1. Top8/11/12-bit partition with smaller uint32 fused local histograms. This
   changes the cache footprint of the prior v21 radix7 approach, but still pays
   scalar scatter and histogram setup costs.
2. AVX-512 binary MSD partition with 8/16/32-key bitonic networks. Compaction
   writes each vector's two groups into contiguous output spans; local networks
   replace branch-heavy tiny comparison sorts. A std::sort-leaf ablation retains
   the identical partition mechanism.

`prototype_probe.cpp`, `prototype_cache.hpp` and `local_screening.csv` retain
the full exploratory screen, including poor binary-input results. All outputs
were oracle-checked. Local wall times varied widely (often multiple-fold), so
this screen selects a candidate and does not support precise latency claims.
Network leaves led the uniform64 screen; cache variants are not promoted. The
prototype algorithm IDs are 0=v17, 1=v24, 2=SIMD network, 3=SIMD stdleaf,
4=cache8, 5=cache11, 6=cache12. The prototype CSV uses default stream precision and is not a qualification artifact.
Qualification seeds are separate and were not used to tune this code.

## Final candidate

The worker partitions one varying bit at a time using AVX-512 masked compress
stores. Depth is bounded by 64 bits. A degenerate partition computes the exact
varying-bit mask, returns for all-equal keys, or skips the common prefix.
Leaves of <=32 keys use conventional bitonic sorting networks, padded with
UINT64_MAX in registers. A distance-zero-style attribution is provided here by
replacing only network leaves with std::sort, not by changing the partitioner.

The public wrapper requires n>=1M and runtime AVX512F+POPCNT support. It probes
the existing v17 entropy condition (>=20 descents and >=60 varying sample bits
in the first 64 keys). Negative probes call the original v17 function directly;
this removes v24's cloned full-dispatch path for random32 and sortedprefix.
Positive probes retain all preceding v17 exact ordered/low-cardinality exits
and bounded reconstruction attempts, then select the SIMD worker. A text-level
inheritance check verifies the prefix. ARM, unsupported x86 CPUs and
RUNE25_DISABLE_SIMD builds directly use frozen v17.

## Correctness argument and resources

At a node, all keys share the already-processed higher-bit prefix. Each vector's
low/high masks partition its lanes exactly once. Output ranges [0,left) and
[right,n) are disjoint; the remaining capacity equals unread input count. Thus
every source key is written once, left==right at completion, and the low child
precedes the high child numerically. Source/destination buffers are disjoint.
Recursion reduces the bit index; if a partition is degenerate, the exact mask
either certifies equality or finds a strictly lower varying bit. At bit<0 all
keys are equal. A child never writes outside its own range in either buffer.

Bitonic compare-exchange stages implement the standard XOR-partner network;
each stage chooses unsigned min/max by the direction bit. Register padding
uses the maximum unsigned key, so retaining the first n lanes preserves exactly
the sorted multiset, including real UINT64_MAX values. Masked memory operations
are issued only when the register's first lane lies inside the input; full loads
only when >=8 keys remain. Final leaf stores write directly to the original
array's corresponding range. The std::sort-leaf ablation shares all partition
invariants. The wrapper's sample chooses a universally correct worker; exact
legacy shortcuts retain their original precondition checks.

For fixed 64-bit words the worker uses O(64n) worst-case operations, O(n) scratch
(8n bytes), and O(64) call depth; small leaf work is bounded by 32 keys. Prefix
variation scans add at most a constant scan per skipped level. The complete
public API retains v17's O(n log n) worst-case bound and O(n) memory. These are
reasoned arguments, not machine-checked proofs.

## Frozen qualification protocol and gates

Six complete public sorts: frozen v17, v24 prefetch128, v25 network, v25 stdleaf,
current pinned VQSort and sequential IPS4o. Native x86 and ARM; sizes 65536,
999999,1000000,4000000; ten modes from v24 (uniform64, random32, K85000, sorted,
alternating, reverse, sparse16, almostsorted, sortedprefix, entropy_prefix).
Five seeds 20301001,20301011,20301021,20301031,20301041; source/order offsets 229/447
times mode index. Optional smoke pilot seed 991009 is separate from qualification
and the prototype seed. One warmup plus three retained measured trials with
randomized algorithm order, each output compared exactly with std::sort; copy,
generator and oracle outside timing. Record runtime SIMD availability per row.
Expected 1200 aggregate rows per ISA, 2400 total, 7200 measured sorts + 2400 warmups.

Fixed gates, evaluated after qualification without retuning:

- Correctness: complete exact-output matrix, full CTest, ASan/UBSan, network
  validation, protected-page tails, forced fallback and baseline-x86 compilation.
- SIMD target eligibility: actual x86 runner must expose AVX512F+POPCNT. An
  unsupported runner verifies fallback but cannot qualify the SIMD performance.
- Benefit: at both 1M/4M x86 uniform64, median same-seed candidate/v17 AND
  candidate/v24 ratios <=0.90, with 5/5 wins against each.
- Network contribution: same target cells, candidate/stdleaf <=0.90 with 5/5 wins.
- Regression: candidate/v17 <=1.05 in all other 78 mode-size-ISA cells.
- Current pinned VQSort superiority: both target ratios <=0.95 with 5/5 wins.
  This remains a bounded comparator gate, not proof of worldwide SOTA.

Medians of paired seed ratios, not ratios of displayed medians. These are
descriptive engineering gates, not significance tests. One run per ISA and
synthetic workloads do not establish broad production readiness. A failing
cell remains reported; no rerunning until green. Do not merge automatically.

## Comparator freshness and prior art (2026-10-21)

Highway default-branch head observed through GitHub:
`3a5844a43b095751a4870ac0b66940d71ab60c38` (2026-10-20).
IPS4o default-branch head: `8d5a97bb4c4048a1eef18b537f191a31462b051a`
(2025-01-15; unchanged from prior experiments). Both are checked after fetching.
The VQSort pin is updated from v24, so compare algorithms within v25 runs.

Closest mechanisms include conventional MSD/LSD hybrids, binary radix
partitioning, Batcher networks, and SIMD compaction. F5 (Arman/Loguinov, 2026)
explicitly studies SIMD MSD sorting, small-sort performance and common-prefix
skipping; it prevents claiming those combinations as new. Its authors' technical
report is https://irl.cse.tamu.edu/people/arif/papers/icde2026-tr.pdf . This work
does not implement their specialized network/mini-sort or adaptive radix design.
VQSort documentation: https://github.com/google/highway/tree/master/hwy/contrib/sort .
Hybrid CPU prior implementation: https://github.com/KirillLykov/int-sort-bmk .
Current research coverage is incomplete (including no measured F5 or Intel
x86-simd-sort baseline), so even a VQSort win cannot establish universal SOTA.
No external implementation is copied or vendored; proprietary license retained.
