// Diagnose the exact earlier duplicate gate on the 5 pinned v16 external seeds.
#include "runesort_v16/rune_sort.hpp"
#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>
using U=std::uint64_t;
int main(){
 constexpr size_t n=4000000, k=81920;
 for(U base:{512003ULL,512009ULL,512011ULL,512021ULL,512047ULL}){
  const U effective=base+3*389; // K81920 is mode index 3 in frozen v16 external generator
  std::mt19937_64 rng(effective);
  std::vector<U>keys(k),a(n);
  for(auto&x:keys)x=rng();
  for(auto&x:a)x=keys[rng()%keys.size()];
  size_t early_duplicates=0;
  const bool short_ok=rune::detail::extended_distinct_reconstruct(a.data(),n,&early_duplicates);
  if(short_ok){std::fputs("unexpected K81920 <= K4096\n",stderr);return 4;}
  const bool pilot=rune::detail::fifth_tier_pilot(a.data(),n);
  std::printf("seed=%llu,rejection_duplicates=%zu,pilot=%d,original_gate=%d,proposed_gate=%d\n",
   (unsigned long long)base,early_duplicates,int(pilot),
   int(early_duplicates>=100),int(early_duplicates>=80));
 }
}
