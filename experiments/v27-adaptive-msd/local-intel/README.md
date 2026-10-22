# Earlier scalar-v27 Intel development comparison

Exact repository public wrappers for source
`d7d7ad0aa8b05d4678fa93db09dd293ddc572456`; four local workloads, 1M/4M,
five seeds, five shuffled trials and one checked warmup. Full-array oracle after
every call. This reduced matrix is exploratory development evidence and does
not replace the full CI qualification. Mode indices differ from the CI suite;
the local seed mapping therefore also differs. Do not pool host latencies.

`target.cpp` is the exact driver; compile it with `-std=c++20 -O3 -march=native
-DNDEBUG -pthread`, source root, and the pinned comparator include paths.
The pinned Highway u64 object was compiled with `-O3 -DNDEBUG -std=c++17 -fPIC
-DHWY_STATIC_DEFINE`; its dispatch/ABI objects and CMake-built libhwy were linked
with `-latomic`. The source and pins are recorded by the parent protocol.
This local link is distinct from CI's complete CMake-built hwy_contrib library.
