# RUNE v9 — Exact 131K-key research prototype

Experimental, stacked on v7 PR #2. Adds a bounded exact 131072-distinct-key table (262144 hash slots) and uses the existing read-only 16K sample's duplicate count to route near-capacity arrays (1150–1700 observed repeats). Failed reconstruction never mutates input; sampling only affects performance. Targets K81K–110K at N=1M. May degrade performance above this range; direct comparisons and negative-control testing are required before acceptance.

Proprietary LICENSE and frozen earlier algorithms unchanged. **Not SOTA-proven. Do not merge without review.**
