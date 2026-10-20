#pragma once
// Isolated experimental workers. Public RUNE sorter is unchanged.
#define rune rune_v17
#include "../runesort_v17/rune_sort.hpp"
#undef rune
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <limits>
#include <memory>
#include <utility>
namespace rune_v19 {
template<bool PerPass> inline void fixed8(std::uint64_t* a,std::size_t n) {
 if(n<2)return;
 if(n>std::numeric_limits<std::uint32_t>::max()){
   rune_v17::detail::fixed_radix<8>(a,n);return;
 }
 std::unique_ptr<std::uint64_t[]> memory(new std::uint64_t[n]);
 std::uint64_t *src=a,*dst=memory.get();
 if constexpr(PerPass){
  for(unsigned d=0;d<8;++d){
   std::uint32_t h[256]{};
   for(std::size_t i=0;i<n;++i)++h[(src[i]>>(8*d))&255u];
   std::uint32_t p=0;
   for(unsigned b=0;b<256;++b){auto c=h[b];h[b]=p;p+=c;}
   for(std::size_t i=0;i<n;++i){auto x=src[i];dst[h[(x>>(8*d))&255u]++]=x;}
   std::swap(src,dst);
  }
 }else{
  std::uint32_t h[8][256]{};
  for(std::size_t i=0;i<n;++i) {
   auto x=src[i];for(unsigned d=0;d<8;++d)++h[d][(x>>(8*d))&255u];
  }
  for(unsigned d=0;d<8;++d){
   std::uint32_t p=0;
   for(unsigned b=0;b<256;++b){auto c=h[d][b];h[d][b]=p;p+=c;}
  }
  for(unsigned d=0;d<8;++d){
   for(std::size_t i=0;i<n;++i){auto x=src[i];dst[h[d][(x>>(8*d))&255u]++]=x;}
   std::swap(src,dst);
  }
 }
 if(src!=a)std::memcpy(a,src,n*sizeof(std::uint64_t));
}
inline void fused32(std::uint64_t* a,std::size_t n){fixed8<false>(a,n);}
inline void perpass32(std::uint64_t* a,std::size_t n){fixed8<true>(a,n);}
}
