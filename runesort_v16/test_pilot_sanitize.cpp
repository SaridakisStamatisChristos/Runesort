#include "rune_sort.hpp"
#define rune rune_v14
#include "runesort_v14/rune_sort.hpp"
#undef rune
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>
using U=uint64_t;
int main(){
 size_t checks=0;
 for(size_t n:{size_t(2000000),size_t(4000000)}){
  for(size_t K:{size_t(73728),size_t(81920),size_t(85000),size_t(100000)}){
   std::mt19937_64 rng(U(91007)+n+K);
   std::vector<U> palette(K),data(n);
   for(auto&x:palette)x=rng();
   for(auto&x:data)x=palette[rng()%K];
   const auto before=data;
   const bool admitted=rune::detail::fifth_tier_pilot(data.data(),n);
   if(data!=before){std::fputs("read-only pilot mutated input\n",stderr);return 2;}
   auto oracle=before;std::sort(oracle.begin(),oracle.end());
   rune::sort(data);
   if(data!=oracle){std::fprintf(stderr,"sort incorrect n=%zu K=%zu pilot=%d\n",n,K,int(admitted));return 3;}
   ++checks;
  }
 }
 // Verify full exact helper rejects an overcapacity domain without mutation
 // instead of allowing a false-positive pilot to corrupt values.
 std::vector<U> distinct(82000);
 for(size_t i=0;i<distinct.size();++i)distinct[i]=U(i)*0x9e3779b97f4a7c15ULL;
 const auto before=distinct;
 if(rune::detail::fifth_distinct_reconstruct(distinct.data(),distinct.size())||distinct!=before){
  std::fputs("exact fallback broke fail-closed contract\n",stderr);return 4;
 }
 std::printf("PASS v16b sanitizer on %zu 2M/4M palette cases + exact fail closed\n",checks);
}
