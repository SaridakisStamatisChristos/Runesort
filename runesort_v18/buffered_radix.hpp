#pragma once
// RUNE v18 experimental stable byte radix with per-bucket staging; not in public dispatcher.
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>
namespace rune_v18 {
template<std::size_t B>
inline void buffered_fixed8(std::uint64_t* a,std::size_t n){
  static_assert(B==8||B==16,"only the specified tested blocks");
  if(n<2)return;
  std::size_t hist[8][256]{};
  for(std::size_t i=0;i<n;++i){
    auto x=a[i];
    for(unsigned d=0;d<8;++d)++hist[d][(x>>(8*d))&255u];
  }
  for(unsigned d=0;d<8;++d){
    std::size_t pos=0;
    for(unsigned k=0;k<256;++k){auto c=hist[d][k];hist[d][k]=pos;pos+=c;}
  }
  std::unique_ptr<std::uint64_t[]> tmp(new std::uint64_t[n]);
  std::uint64_t *src=a,*dst=tmp.get();
  alignas(64) std::array<std::array<std::uint64_t,B>,256> buffer{};
  std::array<unsigned,256> fill{};
  for(unsigned d=0;d<8;++d){
    fill.fill(0);
    auto *pos=hist[d];
    for(std::size_t i=0;i<n;++i){
      const auto x=src[i];
      const unsigned bucket=unsigned((x>>(8*d))&255u);
      buffer[bucket][fill[bucket]++]=x;
      if(fill[bucket]==B){
        std::memcpy(dst+pos[bucket],buffer[bucket].data(),B*sizeof(std::uint64_t));
        pos[bucket]+=B;fill[bucket]=0;
      }
    }
    for(unsigned k=0;k<256;++k){
      if(fill[k]){std::memcpy(dst+pos[k],buffer[k].data(),fill[k]*sizeof(std::uint64_t));pos[k]+=fill[k];}
    }
    std::swap(src,dst);
  }
  if(src!=a)std::memcpy(a,src,n*sizeof(std::uint64_t));
}
}
