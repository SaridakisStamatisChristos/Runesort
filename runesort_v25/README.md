# Experimental v25 unsigned-64 sorter

```cpp
#include "runesort_v25/rune_sort.hpp"

std::vector<std::uint64_t> values = {9, 1, 7};
rune_v25::sort(values);
// Pointer/count overload: rune_v25::sort(values.data(), values.size());
```

Requires C++17. The GCC/Clang x86 implementation checks AVX512F and POPCNT at
runtime; it does not require enabling AVX-512 globally. Eligible inputs of at
least one million keys use SIMD MSD partitioning and bitonic network leaves.
Other inputs and architectures use the frozen v17 sorter. Keys are unsigned
64-bit integers sorted ascending; record stability is not promised. The SIMD
worker allocates 8n bytes of scratch storage. `RUNE25_DISABLE_SIMD` forces the
legacy path; `rune_v25::simd_available()` reports runtime capability.

This is a research candidate. Correctness and fallback qualification passed,
but SIMD performance is **not qualified** and universal SOTA is not established.
See [the measured verdict](../experiments/v25-simd-msd/FINAL_VERDICT.md) and its
raw data before choosing it for a performance-sensitive application.

Build the targeted test through the repository CMake configuration:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target test_v25_simd
ctest --test-dir build -R '^v25_simd$' --output-on-failure
```
