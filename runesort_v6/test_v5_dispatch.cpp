#include "rune_sort.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
using U=uint64_t;
int main(){
 std::mt19937_64 rng(1729);
 for(size_t k:{size_t(64),size_t(4096),size_t(8192),size_t(16384),size_t(32768),size_t(65536)}) {
  std::vector<U> choices(k),a(1000000);
  for(auto& x:choices)x=rng();
  for(auto& x:a)x=choices[rng()%k];
  auto original=a,expected=a;std::sort(expected.begin(),expected.end());
  size_t duplicates=0;
  bool ok=rune::detail::extended_distinct_reconstruct(a.data(),a.size(),&duplicates);
  if((ok && a!=expected)||(!ok && a!=original)){
    std::cerr<<"dispatch observation violates fallback semantics k="<<k<<"\n";return 1;
  }
  if(k==8192 && duplicates<400){std::cerr<<"unexpected 8192 observations\n";return 2;}
  if(k==16384 && duplicates<400){std::cerr<<"unexpected 16384 observations\n";return 3;}
  a=original;rune::sort(a);
  if(a!=expected){std::cerr<<"dispatch sort fails k="<<k<<"\n";return 4;}
 }
 std::vector<U>a(1000000);
 for(size_t i=0;i<a.size();++i)a[i]=i%8192;
 for(size_t i=995000;i<a.size();++i)a[i]=rng();
 auto e=a;std::sort(e.begin(),e.end());rune::sort(a);
 if(a!=e)return 5;
 std::cout<<"PASSED v5 dispatch tests\n";
}
