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
