#pragma once
// Research-only destination lookahead on the frozen v22 base-65536 LSD worker.
// Conventional software prefetch, no algorithmic novelty claim.
#include "../runesort_v22/wide_radix.hpp"
namespace rune_v23 {
template<std::size_t Distance>
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
    std::size_t i=0;
    if constexpr(Distance>0){
#if (defined(__GNUC__) || defined(__clang__)) && !defined(RUNE23_DISABLE_PREFETCH)
      // Distance<n guards subtraction; i<n-Distance guards the source read.
      // The future key has not been scattered, so its bucket cursor is <n.
      // The hint may be early for repeated buckets, but never changes cursors.
      if(n>Distance)for(;i<n-Distance;++i){
        const auto future=src[i+Distance];
        __builtin_prefetch(dst+hist[(future>>shift)&Mask],1,1);
        const auto x=src[i];
        dst[hist[(x>>shift)&Mask]++]=x;
      }
#endif
    }
    for(;i<n;++i){
      const auto x=src[i];
      dst[hist[(x>>shift)&Mask]++]=x;
    }
    std::swap(src,dst);
  }
  // P=4 is even. All values are in the original array.
}
} // namespace rune_v23
