# v23: destination lookahead on frozen v22 radix16

Research question: can conventional software write-prefetch reduce full-width
x86 scatter latency without changing exact ascending uint64 semantics?
No novelty or universal superiority claim. This branch is based on v22
`49dddfa54ce9ef6024d7796f7c9577db899b8f59`, not the rejected v21 designs.

## Fixed experiment, before CI measurement

Two distances only: 32 and 128 source keys. An identical template with distance
zero isolates the hint from code-layout/template effects; frozen v22 radix16
is an additional baseline. Frozen RUNE17 full/fixed8, v22 radix11, pinned VQSort,
and sequential IPS4o provide context. The local-only build explicitly relabels
its two std::sort controls; it cannot generate VQSort/IPS4o rows.

Qualification: native x86 and ARM; 1M and 4M; five fixed new seeds; uniform64,
random32, K85000, sorted, alternating. One warmup plus three measured trials,
randomized algorithm order each round, independent full-array std::sort oracle.
Every warmup and measured output is checked; copies/oracles are outside timing.
CSV retains all three individual times at double round-trip precision.
Provenance and raw CSVs are artifacts. Sanitizers, hint-disabled fallback and
x86 BMI2-off builds are mandatory. CTest registers the new regression test.

Primary mechanism gate (per distance, x86 4M uniform64): median of same-seed
candidate/ablation0 and candidate/frozen-v22 ratios both <=0.95, with 5/5 seed
wins against each. This descriptive gate is not a statistical significance test.
The two distances are competing candidates, not independent replications.
Cross-ISA eligibility additionally requires <=1.05 median candidate/v22 ratio
for every non-ordered workload at each tested size on ARM; failures stay explicit.
Superiority against the pinned VQSort requires <=0.95 median ratio and 5/5 seed
wins on the primary target; a radix16-only gain does not establish this.
No outcome authorizes automatic production integration: full public dispatcher
end-to-end qualification is a separate prerequisite. Sorted/alternating forced
radix timings are controls, not public-RUNE latency.

The first local exploratory pilot reused the first listed seed. Consequently
these five seeds are fixed CI qualification inputs, not a wholly held-out
statistical test set. No candidate or threshold was selected from CI timings.

## Safety/correctness argument

Each digit uses exact counts, exclusive bucket prefixes, stable increasing
source-order stores and four even ping-pong passes. Thus induction over digits
establishes ascending unsigned 64-bit order. Prefetch does not update any state.
For a future key at i+D, its bucket still has at least one unwritten member;
the bucket cursor is in [0,n). i<n-D is evaluated only when n>D, so both source
access and destination pointer expression are valid. The hint can be imprecise
for repeated buckets (cursor can advance before the actual future store), which
is a performance limitation only. n<2 returns before pointer access, including
nullptr with n=0. n>UINT32_MAX falls back to size_t-counter frozen fixed8;
this huge allocation regime is reasoned, not allocation-tested here.

Time O(4n+4*65536), auxiliary storage 8n+4*65536 bytes plus allocator overhead.
No stability promise for the public API; this key-only worker is stable per digit.
No changes to existing algorithm headers, license or public dispatch.

## Mechanism scope and prior art

Software prefetch and LSD radix sorting are established techniques. GCC documents
write intent (rw=1), locality and the requirement that the address expression be
valid: https://gcc.gnu.org/onlinedocs/gcc-14.3.0/gcc/Other-Builtins.html .
This experiment tests the hint; without PMU counters it cannot prove reduced RFO,
cache misses, or memory latency causation. v20 identified scatter as the measured
hot phase; v18 buffering, v19 compact histogram and v21 bucket-local sorting
were already rejected, and are not repeated here.
