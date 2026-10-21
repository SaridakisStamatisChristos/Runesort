#pragma once
// Research v26: stable base-16 MSD scatter with the proven v25 AVX-512
// bitonic network as an optional <=32-key leaf. This is NOT v25's
// one-bit SIMD compressor; benchmark all three mechanisms independently.
#include "../runesort_v25/rune_sort.hpp"
#include <array>
namespace rune_v26 {
namespace detail {
template<bool Network>
inline void node(std::uint64_t* src,std::uint64_t* scratch,
                 std::uint64_t* out,std::size_t n,int shift){
  if(n<=32){
#if RUNE25_X86
    if constexpr(Network){
      if(rune_v25::simd_available()){
        if(n<=8)rune_v25::detail::small<8>(src,out,n);
        else if(n<=16)rune_v25::detail::small<16>(src,out,n);
        else rune_v25::detail::small<32>(src,out,n);
        return;
      }
    }
#endif
    if(src!=out)std::memcpy(out,src,n*sizeof(*out));
    std::sort(out,out+n);
    return;
  }
  // If all 64 bits have been consumed, this bucket contains only equal keys.
  if(shift<0){if(src!=out)std::memcpy(out,src,n*sizeof(*out));return;}
  std::array<std::size_t,16> counts{};
  for(std::size_t i=0;i<n;++i)++counts[(src[i]>>shift)&15u];
  for(unsigned j=0;j<16;++j)if(counts[j]==n){
    // Common prefix: skip all invariant nibbles with an exact full-array mask.
    const auto first=src[0];
    std::uint64_t varied=0;
    for(std::size_t i=1;i<n;++i)varied|=src[i]^first;
    if(!varied){if(src!=out)std::memcpy(out,src,n*sizeof(*out));return;}
    const int next=(63-__builtin_clzll(varied))&~3;
    node<Network>(src,scratch,out,n,next);
    return;
  }
  std::array<std::size_t,16> begin{},cursor{};
  std::size_t total=0;
  for(unsigned j=0;j<16;++j){
    begin[j]=total;total+=counts[j];cursor[j]=begin[j];
  }
  // src and scratch are disjoint; every key lands in its exact digit bucket.
  for(std::size_t i=0;i<n;++i)
    scratch[cursor[(src[i]>>shift)&15u]++]=src[i];
  for(unsigned j=0;j<16;++j)if(counts[j])
    node<Network>(scratch+begin[j],src+begin[j],
                  out+begin[j],counts[j],shift-4);
}
} // namespace detail
template<bool Network=true>
inline void kernel(std::uint64_t* a,std::size_t n){
  if(n<2)return;
  std::unique_ptr<std::uint64_t[]> scratch(new std::uint64_t[n]);
  detail::node<Network>(a,scratch.get(),a,n,60);
}
} // namespace rune_v26
