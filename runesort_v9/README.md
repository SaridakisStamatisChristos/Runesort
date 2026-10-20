# RUNE v9 — Exact 131K-key research prototype

Experimental, stacked on v7 PR #2. Adds a bounded exact 131072-distinct-key table (262144 hash slots) and uses the existing read-only 16K sample's duplicate count to route near-capacity arrays (1150–1700 observed repeats). Failed reconstruction never mutates input; sampling only affects performance. Targets K81K–110K at N=1M. May degrade performance above this range; direct comparisons and negative-control testing are required before acceptance.

Proprietary LICENSE and frozen earlier algorithms unchanged. **Not SOTA-proven. Do not merge without review.**

## Cross-architecture qualification

See [nine-way benchmark](../experiments/v9-crossarch-competition.cpp) and [CI workflow](../.github/workflows/v9-crossarch.yml). Five held-out seeds; separate ARM64 and x86-64 runs, N=100K and 1M. This experiment must not be promoted based only on passing correctness tests; it must beat frozen RUNE v7 with low regression and meet external baseline comparisons.
