# RUNE v26 — 4-bit MSD research candidate

**Status: unqualified research, not a release or state-of-the-art claim.**
Stacked on the frozen v25 candidate; nothing is merged into `main`.

This isolated worker uses a 16-bucket (4-bit) MSD distribution, skipping
common-prefix digits using the exact OR-of-XOR mask and finishing at 32
keys with v25's tested AVX-512 bitonic networks when AVX512F+POPCNT is
available, otherwise `std::sort`. The independent `std::sort` leaf
ablation is `rune_v26::ablation`. The public v26 wrapper keeps v25's
inherited exact v17 ordered/low-cardinality exits and sample-based
high-entropy gate, but permits the scalar nibble worker on all CPUs.

Worst-case on fixed 64-bit unsigned keys: at most 16 nibble levels,
O(16n) distribution work plus bounded-size leaf sorting; O(n) heap scratch
(8n bytes), O(16) recursion depth. This does **not** imply a faster
implementation than v25, VQSort or IPS4o.

API: `rune_v26::sort(uint64_t*, size_t)`,
`rune_v26::sort(std::vector<uint64_t>&)`.
Not stable; no records, floats or signed-key semantics promised.
C++17; no global AVX-512 compilation flag is needed.

## Evidence obligations

1. Full exact-output differential, adversarial, end-of-page, ASan/UBSan
   and no-AVX-512 test suites on both x86 and ARM.
2. Same-host paired timing of v17, v25, v26, v26-stdleaf, current pinned
   VQSort and IPS4o, using the frozen v25 distributions/sizes/seeds.
3. Independent appraisal of any wins and losses. No cherry-picked
   distributions, changed gates or CPU-crossing performance claims.
4. Do not merge this stacked PR without demonstrable, independently
   replicated bounded benefit and no material regressions.
