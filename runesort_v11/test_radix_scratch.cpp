#include "rune_sort.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>
using U=uint64_t;
static int cases=0;
void check(std::vector<U> a,int mode){
 auto expected=a;std::sort(expected.begin(),expected.end());
 if(mode==0)rune::detail::fixed_radix<8>(a.data(),a.size());
 else if(mode==1)rune::detail::fixed_radix<4>(a.data(),a.size());
 else {U mask=0;for(U x:a)mask|=(x^a.front());rune::detail::dense_radix(a.data(),a.size(),mask);}
 if(a!=expected){std::fprintf(stderr,"Incorrect scratch sort: n=%zu mode=%d\n",a.size(),mode);std::exit(3);}
 ++cases;
}
int main(){
 std::mt19937_64 rng(0xDEC0DE07);
 for(size_t n:{size_t(1),size_t(2),size_t(3),size_t(64),size_t(65),size_t(4096),size_t(65536),size_t(300000)}){
  std::vector<U>a(n);
  for(auto&x:a)x=rng();check(a,0);check(a,2);
  for(auto&x:a)x=rng()&0xffffffffULL;check(a,0);check(a,1);check(a,2);
  for(auto&x:a)x=rng()&255;check(a,0);check(a,1);check(a,2);
  for(auto&x:a)x=42;check(a,0);check(a,1);check(a,2);
  for(auto&x:a)x=(rng()&0xffffffULL)|0xFAA0000000000000ULL;check(a,0);check(a,2);
  std::sort(a.begin(),a.end());check(a,0);check(a,2);
 }
 std::printf("PASSED %d scratch differential cases\n",cases);
}
