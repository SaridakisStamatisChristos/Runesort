# RUNE Sort v6 — Fourth exact reconstruction tier (research candidate)

**Status:** Unmerged experimental candidate; RUNE v5 on `main` remains the validated release. Proprietary rights and upstream dependencies remain unchanged.

## Hypothesis
The v5 dispatcher wastes work sorting long low-entropy arrays with approximately
24K–32K distinct 64-bit keys after rejecting its 16K-capacity frequency tier.
A fourth bounded table can be substantially faster than generic radix.

## Mechanism
- Existing 1K and 4K tiers still run first. The 4K tier reports the number of
  duplicate observations before it rejects the 4097th distinct value.
- For `n >= 262144`, counts >=400 attempt v5's 16K tier unchanged.
- Counts in [250,399] attempt the new 32K tier; counts below 250 use the existing
  safe general sorter. These numbers are *heuristics*, not correctness criteria.
- The 32K tier has 131072 slots, a limit of 32768 distinct keys, 32 probes,
  exact equality resolution, and overflow rejection. It does not modify input
  on failure. Successful reconstruction sorts the distinct key-frequency pairs
  before writing the sorted multiset.
- Same API: `rune::sort(uint64_t*,size_t)`; unstable value-only sorting.

## Evidence, limitations, and rejected alternatives
Locally executed on Intel Xeon Platinum 8573C virtual CPU,
GCC 14.2 `-O3 -march=native -DNDEBUG -std=c++17`, 1,000,000 elements,
four independent seeds, five medians per seed, input copying excluded.
These are medians of per-seed medians and are **not** cross-machine proof.

| Distinct values | v5 ms | v6 ms | v5/v6 |
|---|---:|---:|---:|
| 24576 | 24.5311 | 8.5506 | 2.869 |
| 32768 | 25.4963 | 10.7891 | 2.363 |
| 40960 | 31.5024 | 33.5928 | 0.938 |
| uniform 64-bit | 29.8041 | 29.3685 | 1.015 |

K=40960 is a regression in the exploratory tests. K=49152 measurements varied,
so no robust improvement is asserted outside the intended K band.
VQSort and CAFS may outperform v6 on some CPUs or inputs; independent
same-run baselines are required before a merge or SOTA claim.

## Correctness
CMake suite passed 7/7 locally (the six inherited test programs plus new v6
targeted suite). New v6 test: 83 differential cases including 32767, 32768,
32769, misleading prefixes, exact rejected-input preservation, and deliberately
colliding hash keys. Clang 17 ASan+UBSan and portable GCC without BMI2 passed.
No formal proof assistant or exhaustiveness claim.

## Reproduce
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DRUNE_WITH_BOOST_BENCHMARK=OFF
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

The original v5 header is preserved unchanged in `../runesort_v5/rune_sort.hpp`.
