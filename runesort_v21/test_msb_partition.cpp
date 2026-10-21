#include "msb_partition.hpp"
#define rune rune_v17
#include "../runesort_v17/rune_sort.hpp"
#undef rune
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>
using U=std::uint64_t;
static void verify(const std::vector<U>& input){
  auto oracle=input,radix=input,bucket=input,baseline=input;
  std::sort(oracle.begin(),oracle.end());
  rune_v21::bucket_radix7(radix.data(),radix.size());
  rune_v21::bucket_stdsort(bucket.data(),bucket.size());
  rune_v17::detail::fixed_radix<8>(baseline.data(),baseline.size());
  if(oracle!=radix||oracle!=bucket||oracle!=baseline){
    std::fputs("FAIL exact 64-bit MSB partition sort\n",stderr);std::exit(3);
  }
}
int main(){
  size_t cases=0;
  for(size_t n:{size_t(0),size_t(1),size_t(2),size_t(7),size_t(8),
    size_t(9),size_t(63),size_t(64),size_t(65),size_t(127),
    size_t(128),size_t(129),size_t(255),size_t(256),size_t(257),
    size_t(4096),size_t(65536)}){
    for(unsigned mode=0;mode<12;++mode){
      std::mt19937_64 rng(44171+n*271+mode*1019);
      std::vector<U> a(n);
      for(size_t i=0;i<n;++i){
        switch(mode){
        case 0:a[i]=rng();break;
        case 1:a[i]=rng()&UINT64_C(0xffffffff);break;
        case 2:a[i]=i;break;
        case 3:a[i]=n-i;break;
        case 4:a[i]=UINT64_MAX;break;
        case 5:a[i]=i%2?UINT64_MAX:0;break;
        case 6:a[i]=rng()%19;break;
        case 7:a[i]=(rng()%41)*UINT64_C(0x9e3779b97f4a7c15);break;
        case 8:a[i]=(rng()&255ULL)<<56;break;
        case 9:a[i]=(rng()&255ULL)<<48;break;
        case 10:a[i]=(UINT64_C(0xAB)<<56)|(rng()&UINT64_C(0x00ffffffffffffff));break;
        case 11:a[i]=(i%256ULL)<<56 | (rng()&UINT64_C(0x00ffffffffffffff));break;
        }
      }
      verify(a);++cases;
    }
  }
  for(size_t n:{size_t(1000000),size_t(2000000)}){
    for(unsigned mode=0;mode<3;++mode){
      std::mt19937_64 rng(8831+n+mode*19);
      std::vector<U> a(n);
      for(auto&x:a){
        if(mode==0)x=rng();
        if(mode==1)x=rng()&UINT64_C(0xffffffff);
        if(mode==2)x=(rng()&UINT64_C(0x00ffffffffffffff))|UINT64_C(0x7F00000000000000);
      }
      verify(a);++cases;
    }
  }
  std::printf("v21 PASS %zu exact oracle/baseline differential datasets\n",cases);
}
