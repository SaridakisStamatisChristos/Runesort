#pragma once
// RUNE v22 isolated research kernels. Standard LSD radix techniques, no novelty claim.
// The public RUNE sorter and its dispatcher are not modified.
#define rune rune_v17
#include "../runesort_v17/rune_sort.hpp"
#undef rune
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <vector>
namespace rune_v22 {
// 6 stable passes, base 2^11. Fused 6*2048 uint32 histograms fit in 48KiB.
inline void radix11(std::uint64_t* a,std::size_t n){
  if(n<2)return;
  if(n>std::numeric_limits<std::uint32_t>::max()){
    rune_v17::detail::fixed_radix<8>(a,n);return;
  }
  constexpr unsigned B=2048, P=6, Mask=B-1;
  std::vector<std::uint32_t> hist(P*B,0);
  for(std::size_t i=0;i<n;++i){
    const auto x=a[i];
    for(unsigned d=0;d<P;++d)++hist[d*B+((x>>(11*d))&Mask)];
  }
  for(unsigned d=0;d<P;++d){
    std::uint32_t cursor=0;
    for(unsigned k=0;k<B;++k){
      const auto count=hist[d*B+k];hist[d*B+k]=cursor;cursor+=count;
    }
  }
  std::unique_ptr<std::uint64_t[]> scratch(new std::uint64_t[n]);
  auto* src=a;auto* dst=scratch.get();
  for(unsigned d=0;d<P;++d){
    auto* offset=hist.data()+d*B;
    for(std::size_t i=0;i<n;++i){
      const auto x=src[i];
      dst[offset[(x>>(11*d))&Mask]++]=x;
    }
    std::swap(src,dst);
  }
  // P=6 is even. All values are in the original array.
}
// 4 stable base-65536 passes; per-pass 256KiB u32 histogram instead of
// holding 1MiB of 4 fused digit histograms. Each pass rescans the source.
inline void radix16(std::uint64_t* a,std::size_t n){
  if(n<2)return;
  if(n>std::numeric_limits<std::uint32_t>::max()){
    rune_v17::detail::fixed_radix<8>(a,n);return;
  }
  constexpr unsigned B=65536,Mask=B-1;
  std::vector<std::uint32_t> hist(B);
  std::unique_ptr<std::uint64_t[]> scratch(new std::uint64_t[n]);
  auto* src=a;auto* dst=scratch.get();
  for(unsigned d=0;d<4;++d){
    std::fill(hist.begin(),hist.end(),0);
    const unsigned shift=d*16;
    for(std::size_t i=0;i<n;++i)++hist[(src[i]>>shift)&Mask];
    std::uint32_t cursor=0;
    for(unsigned k=0;k<B;++k){
      const auto count=hist[k];hist[k]=cursor;cursor+=count;
    }
    for(std::size_t i=0;i<n;++i){
      const auto x=src[i];
      dst[hist[(x>>shift)&Mask]++]=x;
    }
    std::swap(src,dst);
  }
  // P=4 is even. All values are in the original array.
}
} // namespace rune_v22
