#include "rune_sort.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>

using U = std::uint64_t;
static size_t checks = 0;

// Noncontiguous masks cross byte and high-bit boundaries; they exercise
// the bit-deposit/bit-extract path without depending on BMI2.
static U make_mask(unsigned bits, unsigned offset) {
  U mask = 0;
  for (unsigned i = 0; i < bits; ++i)
    mask |= U(1) << ((i * 29 + offset) & 63);
  return mask;
}
static void verify(std::vector<U> a) {
  auto expected = a;
  std::sort(expected.begin(), expected.end());
  rune::sort(a);
  if (a != expected) {
    std::fprintf(stderr, "v11 compacted radix mismatch: n=%zu\n", a.size());
    std::exit(3);
  }
  ++checks;
}
int main() {
  std::mt19937_64 rng(0x110CA73DULL);
  for (size_t n : {size_t(0),size_t(1),size_t(2),size_t(63),
                   size_t(64),size_t(65),size_t(4096),size_t(65536),
                   size_t(262144),size_t(524288),size_t(524289)}) {
    for (unsigned bits : {9u, 16u, 20u, 22u, 23u}) {
      U mask = make_mask(bits, bits);
      U base = (U(0xF0F0A55A80000001ULL) & ~mask);
      std::vector<U> a(n);
      for (auto &x : a) x = base | rune::detail::deposit(rng(), mask);
      verify(a);
      std::sort(a.begin(), a.end());
      verify(a);
      std::reverse(a.begin(), a.end());
      verify(a);
      // Skewed cardinality: exercise path selection and reconstruct fallback.
      for (auto &x : a) x = base | rune::detail::deposit(rng() & 255, mask);
      verify(a);
    }
  }
  std::printf("PASSED %zu v11 compacted-bit differential cases\n", checks);
}
