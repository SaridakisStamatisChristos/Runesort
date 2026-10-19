# RUNE Sort v5: independently rerun comparison (20 October 2026)

## Verdict

This campaign does **not** establish overall SOTA. It does show substantial low-cardinality advantages against the three measured mainstream algorithms: Boost pdqsort, Boost spreadsort, and `std::sort`. Conversely, pdqsort wins decisively over RUNE v5 on 32,768-distinct keys at N=1M and 4M and on nearly sorted inputs at N=4M. RUNE v4 beats v5 on several dense, random workloads. No direct CAFS, Google Highway VQSort, or IPS4o measurements were executed.

## Exact experiment

- Machine: Intel Xeon Platinum 8573C virtual CPU, Linux x86_64 (shared VM; timing noise possible).
- Compiler: GCC 14.2, `-O3 -march=native -DNDEBUG -std=c++17`.
- Boost headers: version 1.83.0.
- Sort key: `uint64_t`, ascending, full-array in-place sorting; no key/value payloads or stability requirement.
- Algorithms: RUNE v5 (unchanged source from archived release), RUNE v4 (frozen predecessor), Boost pdqsort, Boost spreadsort, libstdc++ std::sort.
- 100K and 1M cases: seeds 271828, 314159, 777777, 8675309; 5 timed repetitions after one warmup. 4M cases: seeds 271828 and 314159; 3 timed repetitions after one warmup.
- Generator / expected sorted reference run outside the timed region. Each algorithm receives an **identical copy** of the input. Array copies excluded from timing, internal allocations included. Timed sort order interleaves to reduce monotonic order bias.
- Every measured result was compared elementwise to a fresh `std::sort` reference; no mismatches.
- Aggregate: median per seed; table values are medians of seed medians. Not a confidence interval, not throughput or statistically verified superiority.
- The benchmark calls the mode 'zipf', but it is a **uniform distribution over 64 powers of two**, not Zipf; use that description.

## Key results (ms; smaller is better)

| n | Input | RUNE v5 | RUNE v4 | pdqsort | spreadsort | std::sort |
|---:|---|---:|---:|---:|---:|---:|
|100,000|8,192 distinct|2.308|2.529|2.597|2.780|7.027|
|1,000,000|8,192 distinct|5.361|26.542|15.767|26.286|67.176|
|4,000,000|8,192 distinct|18.090|136.827|58.594|104.454|270.516|
|1,000,000|16,384 distinct|7.517|25.543|16.976|28.609|75.373|
|4,000,000|16,384 distinct|26.242|134.350|65.410|112.008|288.312|
|1,000,000|32,768 distinct|27.571|26.943|**20.570**|30.650|77.232|
|4,000,000|32,768 distinct|133.868|131.035|**67.958**|123.001|311.122|
|1,000,000|Uniform 64-bit|29.500|**28.232**|34.824|47.925|92.446|
|4,000,000|Uniform 64-bit|155.012|**113.860**|150.325|220.606|405.546|
|1,000,000|Nearly sorted|9.112|**8.925**|10.863|20.021|14.329|
|4,000,000|Nearly sorted|89.332|90.303|**52.996**|87.741|62.392|

## Build and reproduce

Run `./rerun.sh` from this directory. It recompiles the frozen package benchmark, executes all six CMake correctness suites, and runs the 1M-size four-seed comparison. The raw CSV outputs of the original run are included as well as `summary_multisize.csv` for all measured sizes.

## Boundaries and next required tests

CAFS (existing AVX2 low-cardinality frequency sort) and Google Highway VQSort are the two highest-priority independently developed baselines. Source acquisition into this isolated compiler runtime was unavailable, so neither was benchmarked. Competing with Boost sorters is **not** proof of global leadership. Repeat on an isolated physical CPU, pin CPU frequency and affinity, and include CAFS, VQSort, IPS4o, and additional data regimes before a formal SOTA claim.