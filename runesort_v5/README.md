# RUNE Sort v5.0 — adaptive unsigned 64-bit integer sorting

**Status:** executable research prototype, not an established state-of-the-art or novel algorithm.

The v5 contribution is an exact third-tier bounded hash-frequency reconstruction path for `4097 <= K <= 16384` distinct 64-bit keys, selectively dispatched using **duplicate counts observed when the preceding 4096-key table rejects an input**. A rejection never modifies the input, and the radix/comparison fallback remains exact. This improvement complements v4's existing near-sorted, bit-sparse, and smaller-domain paths.

```cpp
#include "rune_sort.hpp"
#include <cstdint>
#include <vector>
int main() {
    std::vector<uint64_t> values = {9, 2, 7, 2, 5};
    rune::sort(values); // values = {2, 2, 5, 7, 9}
}
```

## Build and test

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

Requires C++17. Boost.Sort headers are optional, used only for `runesort_benchmark`. The source exposes `void rune::sort(uint64_t*, size_t)` and a `std::vector<uint64_t>&` overload. This is an **unstable key sorter**, not a stable key/value-record sort. Signed integers, floats, or generic records are not in the v5 API contract.

## Run the reproducible local comparison

```bash
./reproduce.sh 1000000 5
```

The script recompiles using GCC `-O3 -march=native -DNDEBUG -std=c++17`, tests four fixed seeds, and aggregates seed-level medians. It requires Boost.Sort for the benchmark. It compares the **frozen v4 header included in this package** with the v5 header and Boost pdqsort/spreadsort and `std::sort`, on identical inputs and interleaved sort order. Setup/input generation and result verification are outside the timed region; copy-initialization of the working array is excluded, but scratch allocations inside each sort are included. The measurements are sensitive to the CPU, OS scheduler, and virtual machine contention.

The included `release_seed_*.csv` and `release_multiseed_summary.json` were obtained from one AMD EPYC 9V74 KVM virtual CPU environment (GCC 14.2, C++17, 1M elements). Release v5 was rebuilt and rerun after abandoning rejected microoptimizations.

## Main numerical result

Median of four seed medians, milliseconds; **lower is better**:

| Dataset | v4 | v5 | v4/v5 |
|:--|--:|--:|--:|
| K=8192 | 12.5913 | 3.9362 | 3.20x |
| K=16384 | 13.3580 | 5.8967 | 2.27x |
| Uniform 64-bit | 12.9246 | 11.4601 | 1.13x |
| K=512 | 2.2604 | 2.2614 | parity |
| Nearly sorted | 6.0615 | 6.3524 | 0.95x |
| Uniform 32-bit | 11.3168 | 11.8362 | 0.96x |

There is no cross-CPU confirmation or statistical significance claim. Some v5 regressions remain. The 16,384-key bound is only a path capacity; pathological hash collisions can reject a lower-K input and use the fully correct fallback.

## Provenance and negative experiments

- `rune_sort_v4.hpp`: frozen v4 predecessor in a distinct namespace (`rune_v4`) for A/B testing.
- `rune_sort_v1.hpp`, `rune_sort_v2.hpp`, `rune_sort_v3.hpp`: historical baselines used by inherited tests.
- `test_*`: differential, adversarial, boundary, and dispatch tests.
- `research_rejected/`: rejected progressive-frequency and zero-initialization optimization candidates; **not** used in released v5.
- `DESIGN_AND_EVIDENCE.md`: proof obligations, complexity, benchmark details, and prior art.

**Not yet measured in this environment:** direct CAFS, IPS4o, or Google Highway VQSort runs. Existing CAFS work targets overlapping low-cardinality sorting; v5 cannot claim global novelty or SOTA without independent comparisons.

## License

**Proprietary — All Rights Reserved.** Copyright © 2026 Stamatis-Christos Saridakis. No permission to use, copy, modify, or distribute is granted without a separate written license, except as required by law. See the repository [LICENSE](../LICENSE). Third-party dependencies retain their respective licenses.
