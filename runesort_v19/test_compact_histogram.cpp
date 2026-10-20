#include "compact_histogram.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>
using U=std::uint64_t;
static void verify(const std::vector<U>& input){
 auto oracle=input,old=input,fused=input,pass=input;
 std::sort(oracle.begin(),oracle.end());
 rune_v17::detail::fixed_radix<8>(old.data(),old.size());
 rune_v19::fused32(fused.data(),fused.size());
 rune_v19::perpass32(pass.data(),pass.size());
 if(oracle!=old||oracle!=fused||oracle!=pass){std::fputs("wrong\n",stderr);std::exit(3);}
}
int main(){
 size_t tests=0;
 for(size_t n:{size_t(0),size_t(1),size_t(2),size_t(7),size_t(8),size_t(9),size_t(31),
               size_t(63),size_t(64),size_t(65),size_t(255),size_t(256),size_t(257),
               size_t(4096),size_t(65536)}){
  for(unsigned mode=0;mode<10;++mode){
   std::mt19937_64 rng(39191+71*n+991*mode);
   std::vector<U> a(n);
   for(size_t i=0;i<n;++i) {
    switch(mode){
     case 0:a[i]=rng();break;
     case 1:a[i]=rng()&0xffffffffULL;break;
     case 2:a[i]=i;break;
     case 3:a[i]=n-i;break;
     case 4:a[i]=UINT64_MAX;break;
     case 5:a[i]=(i&1)?0:UINT64_MAX;break;
     case 6:a[i]=rng()%7;break;
     case 7:a[i]=(rng()%41)*0x9e3779b97f4a7c15ULL;break;
     case 8:a[i]=(rng()&255ULL)<<56;break;
     case 9:a[i]=(rng()&255ULL)<<24;break;
    }
   }
   verify(a);++tests;
  }
 }
 for(size_t n:{size_t(1048576),size_t(2000000)}){
  std::mt19937_64 rng(579+n);
  std::vector<U> a(n);for(auto&v:a)v=rng();
  verify(a);++tests;
 }
 std::printf("v19: %zu std::sort/full v17 cases passed\n",tests);
}
