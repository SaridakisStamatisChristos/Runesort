#include "rune_sort.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>

using U=std::uint64_t;
static size_t checked=0;
static void verify(std::vector<U> a) {
  auto oracle=a;
  std::sort(oracle.begin(),oracle.end());
  rune::sort(a);
  if(a!=oracle) {
    std::fprintf(stderr,"v13 ARM sparse domain mismatch n=%zu\n",a.size());
    std::exit(3);
  }
  ++checked;
}
int main() {
  std::mt19937_64 g(0x1391AULL);
  for(size_t n:{size_t(0),size_t(1),size_t(63),size_t(64),size_t(65535),
                size_t(262143),size_t(262144),size_t(524288),size_t(524289)}) {
    for(unsigned bits:{15u,16u,17u,20u,22u,23u,24u}) {
      U mask=0;
      for(unsigned i=0;i<bits;++i) mask|=U(1)<<((i*29+7)&63);
      std::vector<U> values(n);
      U common=0xF0F0A50A80000000ULL & ~mask;
      for(auto&x:values)x=common|rune::detail::deposit(g(),mask);
      verify(values);
      std::sort(values.begin(),values.end());
      verify(values);
      std::reverse(values.begin(),values.end());
      verify(values);
      for(auto&x:values)x=common|rune::detail::deposit(g()&255,mask);
      verify(values);
    }
  }
  std::printf("PASSED %zu v13 sparse ARM boundary/differential cases\n",checked);
}
