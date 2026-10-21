#pragma once
// RUNE v22 research: experimental 16-bit LSD radix, 4 passes.
// Production RUNE dispatcher and prior versions are untouched.
#define rune rune_v17
#include "../runesort_v17/rune_sort.hpp"
#undef rune
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <utility>
#include <vector>
namespace rune_v22 {
constexpr unsigned kBins=65536u;
template<bool Fused>
inline void radix16(std::uint64_t* a,std::size_t n){
  if(n<2)return;
  // Counter and prefix arithmetic are exact for this size range.
  // For larger arrays use the existing size_t-counter fixed radix worker.
  if(n>std::numeric_limits<std::uint32_t>::max()){
    rune_v17::detail::fixed_radix<8>(a,n);return;
  }
  std::unique_ptr<std::uint64_t[]> scratch(new std::uint64_t[n]);
  std::uint64_t *src=a,*dst=scratch.get();
  if constexpr(Fused){
    // 1 MiB 4-digit histogram: four random counter writes per key.
    std::vector<std::uint32_t> hist(4u*kBins,0);
    for(std::size_t i=0;i<n;++i){
      auto x=src[i];
      ++hist[0u*kBins+((x>>0)&65535u)];
      ++hist[1u*kBins+((x>>16)&65535u)];
      ++hist[2u*kBins+((x>>32)&65535u)];
      ++hist[3u*kBins+((x>>48)&65535u)];
    }
    for(unsigned d=0;d<4;++d){
      auto* h=hist.data()+d*kBins;
      std::uint32_t p=0;
      for(unsigned j=0;j<kBins;++j){
        auto c=h[j];h[j]=p;p+=c;
      }
    }
    for(unsigned d=0;d<4;++d){
      auto* h=hist.data()+d*kBins;
      const unsigned shift=16*d;
      for(std::size_t i=0;i<n;++i){
        auto x=src[i];
        dst[h[(x>>shift)&65535u]++]=x;
      }
      std::swap(src,dst);
    }
  } else {
    // 256KiB histogram; count each current source pass separately.
    std::vector<std::uint32_t> hist(kBins,0);
    for(unsigned d=0;d<4;++d){
      std::fill(hist.begin(),hist.end(),0u);
      const unsigned shift=16*d;
      for(std::size_t i=0;i<n;++i)++hist[(src[i]>>shift)&65535u];
      std::uint32_t p=0;
      for(unsigned j=0;j<kBins;++j){
        auto c=hist[j];hist[j]=p;p+=c;
      }
      for(std::size_t i=0;i<n;++i){
        auto x=src[i];
        dst[hist[(x>>shift)&65535u]++]=x;
      }
      std::swap(src,dst);
    }
  }
  if(src!=a)std::memcpy(a,src,n*sizeof(std::uint64_t));
}
inline void fused16(std::uint64_t* a,std::size_t n){radix16<true>(a,n);}
inline void perpass16(std::uint64_t* a,std::size_t n){radix16<false>(a,n);}
} // namespace rune_v22
