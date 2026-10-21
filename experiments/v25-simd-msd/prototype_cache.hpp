#pragma once
#include "../../runesort_v23/prefetch_radix.hpp"
namespace rune25_prototype {
template<unsigned TopBits>
void cache_radix(std::uint64_t* a,std::size_t n){
  if(n<2)return;
  if(n>UINT32_MAX){rune_v17::sort(a,n);return;}
  constexpr unsigned B=1u<<TopBits,Shift=64-TopBits,P=(Shift+7)/8;
  std::vector<std::uint32_t> bounds(B+1,0),pos(B);
  for(std::size_t i=0;i<n;++i)++bounds[(a[i]>>Shift)+1];
  for(unsigned b=1;b<=B;++b)bounds[b]+=bounds[b-1];
  std::copy(bounds.begin(),bounds.end()-1,pos.begin());
  std::unique_ptr<std::uint64_t[]> tmp(new std::uint64_t[n]);
  for(std::size_t i=0;i<n;++i){auto x=a[i];tmp[pos[x>>Shift]++]=x;}
  for(unsigned b=0;b<B;++b){
    const auto len=bounds[b+1]-bounds[b];auto*src=tmp.get()+bounds[b];auto*dst=a+bounds[b];
    if(!len)continue;
    if(len<128){std::sort(src,src+len);std::memcpy(dst,src,len*8);continue;}
    std::uint32_t hist[P][256]{};
    for(unsigned i=0;i<len;++i){const auto x=src[i];for(unsigned p=0;p<P;++p)++hist[p][(x>>(8*p))&255];}
    for(unsigned p=0;p<P;++p){std::uint32_t off=0;for(unsigned k=0;k<256;++k){const auto c=hist[p][k];hist[p][k]=off;off+=c;}}
    for(unsigned p=0;p<P;++p){for(unsigned i=0;i<len;++i){auto x=src[i];dst[hist[p][(x>>(8*p))&255]++]=x;}std::swap(src,dst);}
    if(src!=a+bounds[b])std::memcpy(a+bounds[b],src,len*8);
  }
}
}
