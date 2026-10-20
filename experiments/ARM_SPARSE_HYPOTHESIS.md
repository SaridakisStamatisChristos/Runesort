# ARM64 portable sparse-bit sorting experiment

**Research only.** Based on frozen RUNE v7, with the original v7, v6, v5, `main` and proprietary license unchanged.

## Why this may help
A controlled exact-frequency ablation identified an architecture-specific weakness:
on Neoverse-N2 ARM64 at N=1M, RUNE v7 needed ~18.28ms for 16 noncontiguous varying bits, whereas an exact 131K-key frequency table completed in ~10.48ms. On AMD EPYC x86-64 with BMI2, v7 was much faster (~5.90ms) than counting (~10.59ms).

The **smallest sufficient change** is an ARM-only dispatch after the existing full-array variability scan. This uses the proven `varied==16` bitmask, N≥524288, and a large numerical range, and tries v7's existing exact 81,920-key frequency reconstruction. Since at most 2^16=65536 distinct values can exist, the table's capacity is not the limiting factor; any collision-bound rejection fails closed into the original portable compact-bit counting path.

This avoids probabilistic cardinality guesses, preserves x86-64 code paths, and does not modify the data when reconstruction rejects.

## Validation
- The candidate has own differential, adversarial and outlier regression suites in `runesort_arm_candidate/`.
- [ARM64/x86 direct comparison workflow](../.github/workflows/arm-sparse-experiment.yml) compares candidate and frozen v7 on exactly the same cases/seeds, including sparse16, high-bit outliers and random full-width data, with exact output verification. Same-run Boost pdqsort included.
- Compare at 100K and 1M with four fresh seeds and four repetitions/seed.
- No same-run VQSort or IPS4o measured in this first, small mechanistic check. Establish the candidate-to-v7 effect before escalating.

## Acceptance
Require >20% ARM improvement on sparse16 at N=1M across held-out seeds, without material regression on other families or x86. If supported, add direct native VQSort and IPS4o competition before promotion. No algorithmic novelty or universal SOTA claims, and no automatic merge.
