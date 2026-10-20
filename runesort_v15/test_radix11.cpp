#include "rune_sort.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <random>
#include <vector>
using U=uint64_t;
static size_t checks=0;
static void validate(std::vector<U> a){
 auto oracle=a;
 std::sort(oracle.begin(),oracle.end());
 auto direct=a;
 rune::detail::radix11_full(direct.data(),direct.size());
 if(direct!=oracle){std::fputs("v15 direct radix11 failure\n",stderr);std::abort();}
 rune::sort(a);
 if(a!=oracle){std::fputs("v15 dispatcher failure\n",stderr);std::abort();}
 ++checks;
}
int main(){
 std::mt19937_64 rng(0xF015C021ULL);
 for(size_t n:{size_t(2),size_t(63),size_t(64),size_t(1024),
   size_t(65536),size_t(262143),size_t(262144),size_t(262145),size_t(524288),size_t(1000000)}){
  for(unsigned kind=0;kind<11;++kind){
   std::vector<U>a(n);
   for(size_t i=0;i<n;++i){
    U x=rng();
    switch(kind){
     case 0: a[i]=x; break;
     case 1: a[i]=x&0xffffffffULL; break;
     case 2: a[i]=x&65535ULL; break;
     case 3: a[i]=x>>55;break;
     case 4: a[i]=x<<55;break;
     case 5: a[i]=(i&1)?0:std::numeric_limits<U>::max();break;
     case 6: a[i]=U(i)*0x9e3779b97f4a7c15ULL;break;
     case 7: a[i]=~(U(i)*0x9e3779b97f4a7c15ULL);break;
     case 8: a[i]=U(i%2048);break;
     case 9: a[i]=U((i*17)%2097152)<<25;break;
     default:a[i]=x^0x8000000000000000ULL;break;
    }
   }
   validate(a);
   if(n<=65536){std::sort(a.begin(),a.end());validate(a);}
  }
 }
 std::printf("PASS v15 radix11 and sorter exact oracle checks=%zu\n",checks);
}
