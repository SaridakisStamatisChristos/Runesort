#include "radix16.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>
using U=std::uint64_t;
static void verify(const std::vector<U>& v){
  auto oracle=v,baseline=v,fused=v,perpass=v;
  std::sort(oracle.begin(),oracle.end());
  rune_v17::detail::fixed_radix<8>(baseline.data(),baseline.size());
  rune_v22::fused16(fused.data(),fused.size());
  rune_v22::perpass16(perpass.data(),perpass.size());
  if(oracle!=baseline || oracle!=fused || oracle!=perpass){
    std::fputs("v22 radix16 mismatch against independent std::sort\n",stderr);
    std::exit(3);
  }
}
int main(){
  size_t cases=0;
  for(std::size_t n:{std::size_t(0),std::size_t(1),std::size_t(2),std::size_t(3),
    std::size_t(7),std::size_t(8),std::size_t(15),std::size_t(16),std::size_t(17),
    std::size_t(63),std::size_t(64),std::size_t(65),std::size_t(255),std::size_t(256),
    std::size_t(257),std::size_t(511),std::size_t(512),std::size_t(513),
    std::size_t(1023),std::size_t(4095),std::size_t(4096),std::size_t(4097),
    std::size_t(65535),std::size_t(65536)}){
    for(unsigned mode=0;mode<12;++mode){
      std::mt19937_64 g(124123+n*617+mode*919);
      std::vector<U> a(n);
      for(std::size_t i=0;i<n;++i){
        auto x=g();
        switch(mode){
          case 0:a[i]=x;break;
          case 1:a[i]=x&UINT64_C(0xffffffff);break;
          case 2:a[i]=x&UINT64_C(0xffff);break;
          case 3:a[i]=i;break;
          case 4:a[i]=n-i;break;
          case 5:a[i]=UINT64_MAX;break;
          case 6:a[i]=(i&1)?0:UINT64_MAX;break;
          case 7:a[i]=x%3;break;
          case 8:a[i]=(x%81920)*UINT64_C(0x9e3779b97f4a7c15);break;
          case 9:a[i]=(x&255ULL)<<56;break;
          case 10:a[i]=(x&255ULL)<<24;break;
          case 11:a[i]=(x&4095ULL)|UINT64_C(0x7fffffffffff0000);break;
        }
      }
      verify(a);++cases;
    }
  }
  for(auto n:{std::size_t(1000000),std::size_t(2000000)}){
    std::mt19937_64 g(200310+n);
    std::vector<U> a(n);
    for(auto&v:a)v=g();
    verify(a);++cases;
    for(auto&v:a)v&=UINT64_C(0xffffffff);
    verify(a);++cases;
  }
  std::printf("PASS v22 %zu full-array differential cases\n",cases);
}
