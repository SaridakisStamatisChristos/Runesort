# Full RUNE v14 sorting-algorithm benchmarking campaign

**Purpose:** comprehensive reproducible *single-threaded, ascending `uint64_t` value-sort* comparison of frozen RUNE versions and important real-world open implementations on **both native GitHub-hosted ARM64 and x86**.

**This benchmark PR is stacked on the already verified experimental v14 PR #13.** It adds **zero changes to sorter implementations or the proprietary license**. It is a draft until data from all matrix jobs have been analyzed. It is not a claim that RUNE is generally state of the art.

## Algorithm inventory

- Internal: RUNE v14, v13, original independent ARM sparse16 candidate, v7, v6, v5.
- External available on both CPUs: Google Highway VQSort, sequential IPS4o, Boost pdqsort, Boost spreadsort, C++ `std::sort`, C++ `std::stable_sort`.
- External on x86 **only when AVX2 + BMI available**: CAFS with Boost pdqsort fallback. The upstream CAFS implementation has x86 SIMD requirements and must **not** be simulated or silently substituted on ARM.
- Total: 12 algorithms on ARM64, up to 13 on x86. The inventory is a broad named-comparator set, **not literally every sorter ever written**. Comparisons with parallel sorters or GPU sorts are not equivalent to the single-threaded CPU problem contract.

## Input distributions and sizes

Three independent non-overlapping mode groups (33 total):

- `entropy` (12): uniform64, random32, random16, extreme high/low, sparse16 at two scattered bit patterns, contiguous16, split16, sparse17, sparse20, sparse23, sparse24.
- `cardinality` (12): exactly 16/128/1024/4096/8192/16384/32768/40960/65536/73728/81920/131072 **nominal keys**, each chosen uniformly from an independently generated key table. Actual distinct count may be lower on a finite input; the workload label refers to the palette size, not a guaranteed cardinality.
- `ordered` (9): sorted, reversed, rotated, nearly sorted with randomly displaced swaps, locally swapped, constant/equal, organ pipe, sawtooth, alternating extremes.

All are run at `n=100,000`, `1,000,000`, and `4,000,000` for each architecture. Thus 2 × 3 groups × 3 sizes = 18 architecture/group/size combinations. At each distribution, three independent fixed seeds `631003, 631013, 631021` with a per-mode offset, warmup excluded, **three timed repetitions** in shuffled algorithm order. Output of every call is checked against independently computed `std::sort` exactly, including duplicates.

The benchmark's timed span covers the sort **and internal allocations** only. It excludes input construction, cloning, and comparison. It is **one sorting thread** per algorithm, on CPU-matched native `-march=native`. It does not attempt equalize memory budgets, as auxiliary memory is a separate algorithmic trade-off requiring disclosure.

## Provenance and fair competition
- C++20, GCC, `-O3 -march=native -DNDEBUG -pthread` on each runner.
- Pinned upstream snapshots:
  - CAFS [commit `13488afef446ae88e7ad631ec6f862def896094a`](https://github.com/kexibq-official/cafs-lib/commit/13488afef446ae88e7ad631ec6f862def896094a).
  - Highway [commit `c3407ccaaecffa735c911cae3971efa02263211d`](https://github.com/google/highway/commit/c3407ccaaecffa735c911cae3971efa02263211d).
  - IPS4o [commit `8d5a97bb4c4048a1eef18b537f191a31462b051a`](https://github.com/SaschaWitt/ips4o/commit/8d5a97bb4c4048a1eef18b537f191a31462b051a).
  - Boost comes from the Ubuntu 24.04 `libboost-dev` distro package, whose version is logged or may vary by updated runner images. Full OS/CPU/compiler logs saved by GitHub. All RUNE sources are the exact pinned parent branch snapshot.
- The GitHub Actions `checkout` action is pinned to a verified full SHA. External comparator code is never committed to the proprietary repository.
- No ARM-versus-x86 pooled measurements or win-rates; do not call tiny VM-difference margins wins.

## Gates and outputs

`experiments/v14-full-gauntlet.cpp` is the standalone executable.
`experiments/v14-gauntlet-verify.py` enforces every expected case/seed/algorithm exactly once, nonnegative finite timing, three measured repetitions, correct outputs, and architecture-specific CAFS exclusion.
`.github/workflows/v14-full-gauntlet.yml` runs six independent native runner jobs and archives every group/size CSV.

**Research state:** specified and awaiting actual CI. Report per-workload medians and paired seed ratios and distinguish winners from noise, including losses. A "full benchmark" is not a global novelty or globally fastest claim. Verify CI before promoting any comparative ranking.
