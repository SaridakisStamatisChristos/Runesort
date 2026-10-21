#pragma once
// RUNE v21 research: cache-local MSD partition followed by independent bucket sorts.
// Exact unsigned uint64 ascending semantics; not connected to production dispatcher.
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>
namespace rune_v21 {
template<bool ComparatorBuckets>
inline void msb_partition(std::uint64_t* data,std::size_t n){
  if(n<2)return;
  // Disjoint ranges ordered by the most significant digit.
  std::size_t bounds[257]{};
  for(std::size_t i=0;i<n;++i)++bounds[std::size_t(data[i]>>56)+1];
  for(unsigned i=1;i<257;++i)bounds[i]+=bounds[i-1];
  std::size_t write[256];
  for(unsigned k=0;k<256;++k)write[k]=bounds[k];
  std::unique_ptr<std::uint64_t[]> temp(new std::uint64_t[n]);
  for(std::size_t i=0;i<n;++i){
    const auto x=data[i];
    temp[write[unsigned(x>>56)]++]=x;
  }
  // Across buckets: high-byte order. Within each bucket: exact low 56-bit order.
  for(unsigned bucket=0;bucket<256;++bucket){
    const std::size_t begin=bounds[bucket],len=bounds[bucket+1]-begin;
    if(len==0)continue;
    std::uint64_t* scratch=temp.get()+begin;
    std::uint64_t* out=data+begin;
    if(len==1){out[0]=scratch[0];continue;}
    if constexpr(ComparatorBuckets){
      std::sort(scratch,scratch+len);
      std::memcpy(out,scratch,len*sizeof(std::uint64_t));
    }else{
      // Sorting tiny partitions by comparison avoids 7 x 256-bin setup cost.
      if(len<128){
        std::sort(scratch,scratch+len);
        std::memcpy(out,scratch,len*sizeof(std::uint64_t));
        continue;
      }
      // A separate fused seven-digit histogram stays local to one MSB bucket.
      std::size_t hist[7][256]{};
      for(std::size_t i=0;i<len;++i){
        const auto x=scratch[i];
        for(unsigned digit=0;digit<7;++digit)
          ++hist[digit][unsigned((x>>(8*digit))&255u)];
      }
      for(unsigned digit=0;digit<7;++digit){
        std::size_t offset=0;
        for(unsigned j=0;j<256;++j){
          const auto count=hist[digit][j];
          hist[digit][j]=offset;
          offset+=count;
        }
      }
      std::uint64_t* src=scratch;
      std::uint64_t* dst=out;
      for(unsigned digit=0;digit<7;++digit){
        auto* offsets=hist[digit];
        const unsigned shift=8*digit;
        for(std::size_t i=0;i<len;++i){
          const auto x=src[i];
          dst[offsets[unsigned((x>>shift)&255u)]++]=x;
        }
        std::swap(src,dst);
      }
      // Seven swaps, starting temp->data, leave all final elements in data.
    }
  }
}
inline void bucket_radix7(std::uint64_t* a,std::size_t n){msb_partition<false>(a,n);}
inline void bucket_stdsort(std::uint64_t* a,std::size_t n){msb_partition<true>(a,n);}
} // namespace rune_v21
