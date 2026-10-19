# RUNE Sort

**RUNE Sort v5** is an experimental, adaptive sorter for ascending `uint64_t` arrays in C++17. It combines range/bit-domain specializations, bounded exact frequency reconstruction, ordered-run handling and an exact fallback.

> **Research status:** Tested prototype. Neither global state-of-the-art performance nor algorithmic novelty has been established. Direct CAFS, Google Highway VQSort and IPS4o comparisons remain outstanding.

## Quick start

```cpp
#include "runesort_v5/rune_sort.hpp"
#include <cstdint>
#include <vector>
int main() {
    std::vector<std::uint64_t> values{9, 2, 7, 2, 5};
    rune::sort(values);
}
```

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Repository contents

- [Algorithm header](runesort_v5/rune_sort.hpp) — v5 implementation.
- [Tests](runesort_v5/) — differential, adversarial, and dispatch checks.
- [Technical evidence](runesort_v5/DESIGN_AND_EVIDENCE.md) — correctness arguments, limitations, and rejected hypotheses.
- [Independent comparison report](benchmarks/competition_2026-10-20/RUN_REPORT.md) — measured 100K/1M/4M comparisons with C++ `std::sort`, Boost `pdqsort`, Boost `spreadsort`, and frozen RUNE v4.
- [Raw measurements](benchmarks/competition_2026-10-20/summary_multisize.csv).

## Scope and reproducibility

Input: unsigned 64-bit integers, ascending order; **not stable**. This API does not promise sorting of arbitrary records or signed floating-point values. Run local correctness tests before citing benchmark results; recorded timings are environment-dependent and do not establish universal superiority.

## License

**Proprietary — All Rights Reserved.** Copyright © 2026 Stamatis-Christos Saridakis. Source visibility does not grant rights to use, copy, modify, or distribute the code. See [LICENSE](LICENSE) for the full terms. Third-party dependencies remain governed by their own licenses.
