# RUNE v10 Algorithm Foundry — exploratory candidate bank

Local machine: AMD EPYC 9V74 virtualized, Debian GCC 14.2, `-O3 -march=native -DNDEBUG -std=c++17`; one million unsigned 64-bit keys. All standalone sort output verified against C++ std::sort. These are unpinned VM exploratory measurements, **not** the CI decision metric or global SOTA results. Histograms were fused into an initial counting scan; working-array copying excluded from timed interval and scratch initialization included.

## Mechanisms evaluated
1. **8-bit LSD radix (baseline):** 8 stable passes, 256 bins/pass. Controls most RUNE full-width fallback workloads.
2. **10-bit LSD radix:** seven passes, 1024 bins for first six digits, 16 for final digit. Smaller number of scatters, larger histograms. Locally promising at some K~80K–130K, while any speedup remains CPU/data-sensitive.
3. **11/13/16-bit LSD radix:** Tested higher bin counts in separate standalone loops. Cache and histogram costs cause inconsistent regressions, particularly with uniformly random keys. **Rejected as global fallback replacements.**
4. **MSD high-bit partition + per-bucket std::sort:** Correct by ordering high-bit buckets first, but much slower (~29–43ms on selected 1M-key inputs, vs ~13–20ms radix). **Rejected.**
5. **Guarded v10:** Use the seven-pass 10-bit sorter only when v7's exact 4K-tier rejection yields [40,180] duplicates, `n>=524288`, and the 64-element high-entropy scout sees at least 20 descents/60 varied bits; this is a *performance-only* gate. Failed exact-frequency tiers remain read-only.

## Verification and caution
Standalone C++ sort variants were differential-checked on small random, repeated, 32-bit truncated and narrow-key cases; all observed outputs correct. The local measurements had about 3–4 independent seeds depending on prototype and 3–4 measured iterations. Some results differed materially between repeated experiments, especially 11-bit radix. They should not support a definitive superiority claim. The decisive evidence is [the same-run frozen v7 CI benchmark](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39399619326) and the independent [Highway/IPS4o CI qualification](https://github.com/SaridakisStamatisChristos/Runesort/actions/runs/39399873667).

## Prior art and novelty
Wider LSD digits, cache-sized histograms, and fused histograms are established engineering options, not claims of a new asymptotic algorithm:
- https://github.com/eloj/radix-sorting
- https://github.com/clausecker/radsort
- https://github.com/klauspost/radsort

**Evidence state:** Surviving C++ candidate; not proved or SOTA-qualified. Do not merge without explicit approval.
