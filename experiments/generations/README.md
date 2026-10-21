# Same-host historical RuneSort generation competition (research only)

Independent v26b stacked branch, **no algorithm source modifications**. This
measures the full public `sort` entrypoints v5/v6/v7/v13/v14/v16/v17/v24/v25/v26b,
plus `std::sort`. Diagnostic radix-only workers v22 radix11, v22 radix16,
v23 prefetch128 and v26b nibble are **not public-dispatch implementations**;
compare them *as workers only*, not as proof of public-sort superiority.

Each of 10 exact distributions from v26 is tested at 1M and 4M
unsigned 64-bit keys, five frozen seeds, one verified warmup, three
timed repetitions in shuffled algorithm order. Every complete output
matches the independent reference `std::sort`. Generation and copying,
oracle construction, and result comparison are outside the timed section.
One CSV per size, native x86_64 and aarch64 runners, source SHA/CPU/compiler
recorded, deterministic complete matrix of 1,500? rows per architecture
(actual: 15 algorithms × 10 distributions × 2 sizes × 5 seeds = 1,500).
Python summary rejects duplicates, missing cells, invalid timings and
inconsistent trial summary. Ratios are median *paired seed ratios*, not
ratios of independently reported medians. Lower latency is better.

**Important:** Native GitHub x86 often lacks AVX-512; v26b then deliberately
falls back to v17. Such measurements assess dispatch/fallback and historical
alternatives but cannot validate v26b SIMD acceleration. Hardware and
experimental scope must accompany any superiority claim. No merge on
benchmark outcome alone.
