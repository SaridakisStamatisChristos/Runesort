#include "rune_sort.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
using U=uint64_t;
static size_t cases=0;
static void check(std::vector<U> a,bool direct=true){
 auto initial=a,ref=a;std::sort(ref.begin(),ref.end());
 if(direct){bool ok=rune::detail::sixth_distinct_reconstruct(a.data(),a.size());
   if((ok&&a!=ref)||(!ok&&a!=initial)){std::cerr<<"sixth reconstruction failure\n";std::exit(2);}
   a=initial;}
 rune::sort(a);if(a!=ref){std::cerr<<"sort mismatch\n";std::exit(3);}++cases;
}
int main(){
 std::mt19937_64 rng(20261020ULL);
 for(size_t n:{size_t(0),size_t(1),size_t(65),size_t(65536),size_t(262144),size_t(524288),size_t(1000000)}){
  for(size_t k:{size_t(16384),size_t(73728),size_t(81920),size_t(81921),size_t(90112),size_t(98304),size_t(110592),size_t(131071),size_t(131072),size_t(131073),size_t(147456)}){
   std::vector<U> keys(k),a(n);for(auto&x:keys)x=rng();for(auto&x:a)x=keys[rng()%k];
   check(a,n>=65536);
   if(n>=524288){for(size_t i=0;i<std::min(n,k);i+=37)a[i]=rng();check(a,false);}
  }std::vector<U>a(n);for(auto&x:a)x=rng();check(a,false);
 }
 std::vector<U> colliders;
 for(U x=0;x<17000000 && colliders.size()<34;++x){U h=(x^(x>>32))*0x9e3779b97f4a7c15ULL;h^=h>>33;if((h>>(64-18))==251)colliders.push_back(x);}
 if(colliders.size()<34){std::cerr<<"collision set missing\n";return 4;}
 std::vector<U>a(200000);for(size_t i=0;i<a.size();++i)a[i]=colliders[i%34];
 auto before=a;if(rune::detail::sixth_distinct_reconstruct(a.data(),a.size())||a!=before){std::cerr<<"collision safety failure\n";return 5;}
 check(a,false);std::cout<<"PASSED "<<cases<<" sixth tier tests\n";
}
