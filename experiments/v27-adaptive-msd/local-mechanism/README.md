# Development pilot, not qualification

On the local Intel Xeon Platinum 8573C, compare scalar frequency reconstruction
with AVX-512 nonzero enumeration and masked stores. One input seed (707601),
N=1M/4M and sparse masks of 16/17/20 bits, one warmup plus five alternating-order
measurements. Every complete output is checked exactly against std::sort.
The `alg` field is 0 for scalar and 1 for SIMD. Timings include histogram
allocation/counting and reconstruction, and exclude the common mask certificate.

The pilot informed the <=8 average-multiplicity reconstruction gate; it is
**development data**, not held-out evidence or an independent speed claim.
It shows negative cells too: the 4M/17-bit SIMD pilot was slower. No assumption
of statistical significance or stable CPU frequency is made. Compile with
`g++ -O3 -std=c++17 -I. reconstruction.cpp` on an AVX-512F/BMI2-capable host;
this standalone pilot intentionally lacks runtime ISA dispatch.
