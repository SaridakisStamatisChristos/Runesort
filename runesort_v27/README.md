# RUNE v27 — exact compact-domain counting

C++17, single-threaded ascending `uint64_t` sorting. The public entry point is
`rune_v27::sort(pointer, length)` or `rune_v27::sort(vector)`. It is not stable
and does not sort arbitrary records. A valid writable range is required;
`nullptr, 0` is accepted. The existing proprietary license applies.

```cpp
#include "runesort_v27/rune_sort.hpp"
std::vector<std::uint64_t> a{9, 2, 7, 2, 5};
rune_v27::sort(a);
```

The change targets disordered compact bit domains. Three 64-key windows select
an attempt. An exact full-array varying-bit mask then certifies every input.
Domains of 12–20 varying bits use a `uint32_t` histogram, bounded to 4 MiB and
no more than two bins per input key. Contiguous masks use shifts on all
architectures; sparse masks require runtime-detected BMI2. All other inputs
use the unchanged v17 public sorter. `RUNE27_DISABLE_BMI2` exercises the portable
fallback. No AVX-512 or BMI2 is required to compile the public header.

The sample cannot establish a domain restriction. A hidden outlier outside
all sampled windows rejects the attempt without modifying the array. The
input length must fit `uint32_t` before counters are used. Allocation completes
before any reconstruction writes. There is no global production state.

For a certified mask M, extraction enumerates variable positions in increasing
bit order. Every input is exactly `constant | deposit(code, M)`, and this map
is strictly increasing in `code`. Histograms count every key exactly once;
ascending reconstruction therefore preserves the multiset and sorts it.
Unsuccessful certification is read-only. The counter bound `n <= UINT32_MAX`
ensures no per-key frequency can overflow. The fast path costs O(n + 2^b) time
and O(2^b) auxiliary space for b <= 20; the inherited fallback retains its own
O(n log n) worst-case bound and O(n) auxiliary space. This is conventional
counting/bit-domain compression, with no algorithmic novelty claim.

`byte_msd.hpp` is a separately labeled experimental worker, not a public
replacement. It reduces radix depth and uses the inherited network leaves,
but local timing pilots were noisy and did not establish superiority. Its
portable, SIMD-disabled and direct-kernel correctness paths remain testable.

See `experiments/v27-adaptive-msd/PLAN.md` for frozen performance gates and
comparison methodology. No universal SOTA claim follows from this change.
