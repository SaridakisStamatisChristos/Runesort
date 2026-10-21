#pragma once
// RUNE v21 research only: exact adaptive MSD bucket partition, followed by local std::sort.
// This is an experimental, conventional hybrid algorithm, NOT a production dispatcher change.
#define rune rune_v17
#include "../runesort_v17/rune_sort.hpp"
#undef rune
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>
namespace rune_v21 {
template<unsigned BucketBits>
inline void msd_bucket_local(std::uint64_t* a,std::size_t n) {
  static_assert(BucketBits==8||BucketBits==12,"only qualified bucket widths");
  if(n<2)return;
  // Find the most significant varying bit. Above it, all key prefixes
  // are identical: the masked bucket code is therefore globally monotone.
  std::uint64_t low=a[0],high=a[0];
  for(std::size_t i=1;i<n;++i){
    if(a[i]<low)low=a[i];
    if(a[i]>high)high=a[i];
  }
  if(low==high)return; // every element is identical
  const unsigned highest=63u-static_cast<unsigned>(__builtin_clzll(low^high));
  const unsigned shift=highest+1>=BucketBits ? highest+1-BucketBits : 0;
  constexpr std::size_t Bins=std::size_t(1)<<BucketBits;
  constexpr std::uint64_t Mask=Bins-1;
  std::vector<std::size_t> count(Bins,0);
  for(std::size_t i=0;i<n;++i)++count[(a[i]>>shift)&Mask];
  std::vector<std::size_t> begin(Bins+1,0);
  for(std::size_t b=0;b<Bins;++b)begin[b+1]=begin[b]+count[b];
  // Each output cell is filled exactly once before it is read.
  std::unique_ptr<std::uint64_t[]> scratch(new std::uint64_t[n]);
  auto position=begin;
  for(std::size_t i=0;i<n;++i){
    const auto value=a[i];
    scratch[position[(value>>shift)&Mask]++]=value;
  }
  // Every key in bucket b is smaller than every key in bucket b+1.
  // The local comparison sort handles ties, sparse domains and any bits
  // omitted by the high-bit bucket projection.
  for(std::size_t b=0;b<Bins;++b){
    const auto first=begin[b],last=begin[b+1];
    if(last-first>1)std::sort(scratch.get()+first,scratch.get()+last);
  }
  std::memcpy(a,scratch.get(),n*sizeof(std::uint64_t));
}
inline void bucket8(std::uint64_t* a,std::size_t n){msd_bucket_local<8>(a,n);}
inline void bucket12(std::uint64_t* a,std::size_t n){msd_bucket_local<12>(a,n);}
} // namespace rune_v21
