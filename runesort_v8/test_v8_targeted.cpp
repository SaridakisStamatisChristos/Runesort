#include "rune_sort.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
using U=uint64_t;
static size_t checks=0;
static void exact(std::vector<U> a) {
  auto ref=a;std::sort(ref.begin(),ref.end());
  rune::sort(a);
  if(a!=ref) {std::cerr<<"v8 wrong sorting at case "<<checks<<" n="<<a.size()<<"\n";std::exit(3);}
  ++checks;
}
static void bounded_reconstruction(std::vector<U> a){
  auto before=a,ref=a;std::sort(ref.begin(),ref.end());
  bool accepted=rune::detail::fifth_distinct_reconstruct(a.data(),a.size());
  if((accepted&&a!=ref)||(!accepted&&a!=before)){
    std::cerr<<"v8 fifth tier violated exact or fail-closed contract\n";std::exit(4);
  }
  ++checks;
}
int main(){
 std::mt19937_64 rng(0x8282b0dULL);
 for(size_t k:{size_t(40960),size_t(65536),size_t(73728),size_t(74752),
               size_t(76800),size_t(77824),size_t(79872),size_t(81920),
               size_t(81921),size_t(90112),size_t(98304)}){
   std::vector<U> choices(k),a(1000000);
   for(auto&x:choices)x=rng();
   for(auto&x:a)x=choices[rng()%k];
   bool pilot=rune::detail::fifth_tier_pilot(a.data(),a.size());
   (void)pilot;
   exact(a);bounded_reconstruction(a);
   // Misleading order: pilot may be wrong; completed exact sort must not.
   for(size_t i=0;i<16384;i+=13)a[i]=rng();
   exact(a);
 }
 for(size_t n:{size_t(0),size_t(1),size_t(63),size_t(64),size_t(65535),
              size_t(65536),size_t(524287),size_t(524288),size_t(525000)}){
   std::vector<U>a(n);
   for(auto&x:a)x=rng();
   exact(a);
   if(n){std::sort(a.begin(),a.end());exact(a);
     std::rotate(a.begin(),a.begin()+n/3,a.end());exact(a);}
 }
 // Attempt a deliberately collision-dense sample. The pilot must terminate
 // and the full algorithm must still output the exact sorted multiset.
 std::vector<U> collision;
 for(U x=0;x<5000000&&collision.size()<70;++x){
   U h=(x^(x>>32))*0x9e3779b97f4a7c15ULL;h^=h>>33;
   if(h>>(64-15)==135)collision.push_back(x);
 }
 if(collision.size()<70){std::cerr<<"cannot produce collision witness\n";return 9;}
 std::vector<U>a(600000);
 for(size_t i=0;i<a.size();++i)a[i]=collision[i%collision.size()];
 const auto before=a;
 (void)rune::detail::fifth_tier_pilot(a.data(),a.size());
 if(a!=before){std::cerr<<"pilot changed input\n";return 10;}
 exact(std::move(a));
 // Sparse, noncontiguous sixteen-bit variable mask regression guard.
 U mask=0;for(int bit:{0,4,9,13,18,23,27,32,36,41,45,49,53,57,61,63})mask|=U(1)<<bit;
 a.resize(1000000);
 for(auto&x:a)x=rune::detail::deposit(rng()&65535,mask);
 exact(std::move(a));
 std::cout<<"PASSED "<<checks<<" boundary / fail-closed cases\n";
}
