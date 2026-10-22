# RUNE Sort

Adaptive, single-threaded ascending `uint64_t` sorting in C++17. The current
API combines exact compact-domain counting with the v17 general sorter.
The legacy v5 API remains available for compatibility.

## Quick start

```cpp
#include <runesort/rune_sort.hpp>
#include <cstdint>
#include <vector>
int main() {
    std::vector<std::uint64_t> values{9, 2, 7, 2, 5};
    runesort::sort(values);
    // Also: runesort::sort(values.data(), values.size());
}
```

Add this repository's `include/` directory to the include path. CMake consumers
can link the interface target `runesort::current` after `add_subdirectory`.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DRUNE_WITH_BOOST_BENCHMARK=OFF
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Current implementation

Disordered domains with 12–20 varying bits can use a certified frequency table
bounded to 4 MiB. Contiguous domains work portably; sparse domains use
runtime-detected BMI2. Sparse low-multiplicity tables can reconstruct with
runtime-detected AVX-512 masked stores. Other inputs use the unchanged v17
sorter and its exact ordered, range, cardinality and radix paths. The build
does not require either ISA. Unsupported hardware uses portable fallbacks.

A sample chooses an attempt; a full-array mask certifies every key before
reconstruction. Counter bounds, hidden outliers, disabled ISA paths and
protected output boundaries are covered by differential tests and sanitizers.
The sorting headers are imported verbatim from the measured research source;
see [source provenance](benchmarks/v27/SOURCE_PROVENANCE.json).

## Performance and evidence

Qualification compares complete public sort calls against pinned Highway
VQSort, numpy/x86-simd-sort, IPS4o, CAFS, Boost pdqsort/spreadsort and std::sort.
It retains all 33 workloads at 100K, 1M and 4M keys, five seeds, five chronological
trials and a checked warmup. Every output is compared exactly with std::sort.

The frozen qualification passed all 198 regression guards and all four target
benefit gates. It recorded 10,890 exact-oracle observations, 54,450 timed sorts
and 10,890 verified warmups. Native x86/ARM correctness and leak-enabled
ASan/UBSan jobs all passed. In 24 defined CPU/workload/size cells, RUNE beat
every compared public sorter by the fixed margin with five of five seed wins.

Example: native Intel Xeon Platinum 8573C, four million sparse-domain keys.
These are medians in milliseconds from one host; gate ratios are seed-paired.

| Domain | RUNE v27 | RUNE v17 | VQSort | x86-simd-sort |
|---|---:|---:|---:|---:|
| 17 varying bits | 11.37 | 78.35 | 36.66 | 36.47 |
| 20 varying bits | 17.56 | 102.45 | 35.74 | 36.53 |

[Full source-specific verdict and raw evidence](https://github.com/SaridakisStamatisChristos/Runesort/blob/20870417c5f0be9f01347b91fdbe03ba1eb1dd29/experiments/v27-adaptive-msd/FINAL_VERDICT.md).

Performance depends on CPU, input domain and size. Compact-domain gains do not
establish unrestricted random64 leadership or universal SOTA. Counting sort,
bit extraction and SIMD reconstruction are established techniques; no algorithmic
novelty claim is made. Read the [evidence guide](benchmarks/v27/README.md) for
source-specific results, limits and reproduction.

## API contract

Input is a valid writable array of unsigned 64-bit integers; `nullptr, 0` is
accepted. Output is ascending and preserves the multiset. This is not a stable
record sort and does not accept arbitrary comparators, signed integers or
floating-point inputs. Allocation failures may throw; the counting path allocates
before reconstruction. It uses O(n + 2^b) time for certified b<=20 domains, and
the fallback retains O(n log n) worst-case time and O(n) auxiliary memory.

`RUNE27_DISABLE_BMI2` disables sparse counting; `RUNE27_DISABLE_SIMD` preserves
BMI2 counting while disabling SIMD reconstruction. The legacy interface is
`#include "runesort_v5/rune_sort.hpp"` with `rune::sort`.

## License

**Proprietary — All Rights Reserved.** Copyright © 2026 Stamatis-Christos
Saridakis. Source visibility does not grant rights to use, copy, modify or
distribute the code. See [LICENSE](LICENSE). External benchmark implementations
retain their upstream licenses and are not bundled into the sorting API.

## Research history

- [Current qualification and reproducible harness — PR #31](https://github.com/SaridakisStamatisChristos/Runesort/pull/31)
- [Prior 33-workload historical competition — PR #30](https://github.com/SaridakisStamatisChristos/Runesort/pull/30)
- [Archived v5 independent comparison](experiments/direct-sota-evidence/REPORT.md)

Historical research branches and negative results remain intact. A separately
measured byte-MSD worker is experimental and is not part of this public API.
