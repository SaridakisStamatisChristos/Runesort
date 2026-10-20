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
static size_t total=0;
static void validate(size_t n,size_t k,U seed){
 std::mt19937_64 g(seed);
 std::vector<U> keys(k);
 for(auto&x:keys)x=g();
 std::vector<U>a(n);
 for(auto&x:a)x=keys[g()%keys.size()];
 const auto oracle=[&](){auto x=a;std::sort(x.begin(),x.end());return x;}();
 auto b=a;
 rune::sort(a);
 rune_v14::sort(b);
 if(a!=oracle || b!=oracle){
  std::fprintf(stderr,"FAIL n=%zu K=%zu seed=%llu\n",n,k,(unsigned long long)seed);
  std::exit(4);
 }
 ++total;
}
int main(){
 // Construct an exact 1500-repeat 16,384-sample counterexample:
 // old fixed 1600-repeat gate rejects at both 1M and 2M, whereas
 // new n-adaptive gate admits at 2M and leaves 1M unchanged.
 for(size_t n:{size_t(1000000),size_t(2000000)}){
  std::vector<U> a(n,123456);
  for(size_t i=0;i<16384;++i) a[(i*n)/16384]=U(i%14884);
  const bool new_result=rune::detail::fifth_tier_pilot(a.data(),a.size());
  const bool old_result=rune_v14::detail::fifth_tier_pilot(a.data(),a.size());
  if(new_result!=(n>=2000000)||old_result){
   std::fprintf(stderr,"pilot counterexample fail n=%zu new=%d old=%d\n",
       n,int(new_result),int(old_result));
   return 5;
  }
  ++total;
 }
 for(size_t n:{size_t(100000),size_t(1000000),size_t(2000000),size_t(4000000)}){
  for(size_t k:{size_t(65536),size_t(73728),size_t(81920),size_t(85000),
    size_t(90000),size_t(95000),size_t(100000),size_t(131072)}){
    validate(n,k,U(52016)+n+k);
  }
 }
 std::printf("PASS v16 pilot threshold witness and %zu exact v14/v16 oracle cases\n",total);
}
