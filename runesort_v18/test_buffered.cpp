#include "buffered_radix.hpp"
#define rune rune_v17
#include "../runesort_v17/rune_sort.hpp"
#undef rune
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <random>
#include <vector>
using U=std::uint64_t;
static void check(const std::vector<U>& v){
  auto oracle=v,baseline=v,b8=v,b16=v;
  std::sort(oracle.begin(),oracle.end());
  rune_v17::detail::fixed_radix<8>(baseline.data(),baseline.size());
  rune_v18::buffered_fixed8<8>(b8.data(),b8.size());
  rune_v18::buffered_fixed8<16>(b16.data(),b16.size());
  if(oracle!=baseline||oracle!=b8||oracle!=b16){
    std::fputs("failed v18 exact oracle\n",stderr);std::exit(2);
  }
}
int main(){
  size_t cases=0;
  for(size_t n:{size_t(0),size_t(1),size_t(2),size_t(7),size_t(8),size_t(9),
     size_t(31),size_t(63),size_t(64),size_t(65),size_t(255),size_t(256),
     size_t(257),size_t(4096),size_t(65536)}){
    for(unsigned mode=0;mode<10;++mode){
      std::mt19937_64 rng(9139+n*71+mode*199);std::vector<U> v(n);
      for(size_t i=0;i<n;++i){
        switch(mode){
          case 0:v[i]=rng();break;
          case 1:v[i]=rng()&0xffffffffULL;break;
          case 2:v[i]=i;break;
          case 3:v[i]=n-i;break;
          case 4:v[i]=UINT64_MAX;break;
          case 5:v[i]=(i&1)?0:UINT64_MAX;break;
          case 6:v[i]=rng()%7;break;
          case 7:v[i]=(rng()%41)*0x9e3779b97f4a7c15ULL;break;
          case 8:v[i]=rng()&0xff00ULL;break;
          case 9:v[i]=(rng()&255ULL)<<56;break;
        }
      }
      check(v);++cases;
    }
  }
  for(size_t n:{size_t(1000000),size_t(2000000)}){
    std::mt19937_64 rng(11921+n);
    std::vector<U> v(n);for(auto&x:v)x=rng();
    check(v);++cases;
  }
  std::printf("PASS %zu exact baseline/v18/std::sort cases\n",cases);
}
