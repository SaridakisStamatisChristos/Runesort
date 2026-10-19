#include "rune_sort.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
#include <cstdlib>
using U=uint64_t;
int main(){
 std::mt19937_64 rng(0x8888aabbffff1234ULL);
 size_t count=0;
 for(size_t n:{size_t(0),size_t(1),size_t(2),size_t(63),size_t(64),size_t(65),size_t(129),size_t(512),size_t(4096),size_t(32768),size_t(65536),size_t(65537),size_t(100000)}){
  for(int i=0;i<40;++i){
   std::vector<U> a(n);
   U span=(i%6==0?~U(0):i%6==1?0xffffffffULL:i%6==2?0xffffffffffffULL:i%6==3?0xffffffffffffffULL:i%6==4?0xffff:255);
   for(auto&v:a)v=rng()&span;
   if(i%9==0 && n)a.back()=~U(0);
   if(i%9==1 && n)a[0]=~U(0);
   if(i%9==2)std::sort(a.begin(),a.end());
   auto ref=a;std::sort(ref.begin(),ref.end());
   rune::sort(a);
   if(a!=ref){std::cerr<<"FAIL scratch at "<<n<<" iter="<<i<<"\n";return 1;}
   ++count;
  }
 }
 for(size_t n:{size_t(1),size_t(255),size_t(256),size_t(4096),size_t(25000)}){
  for(int p=0;p<80;++p){
   std::vector<U>a(n);for(auto&v:a)v=rng();
   auto ref=a;std::sort(ref.begin(),ref.end());
   rune::detail::fixed_radix<8>(a.data(),a.size());
   if(a!=ref){std::cerr<<"FAIL 8-pass scratch\n";return 2;}
   for(auto&v:a)v=rng()&0xffffffffULL;
   ref=a;std::sort(ref.begin(),ref.end());
   rune::detail::fixed_radix<4>(a.data(),a.size());
   if(a!=ref){std::cerr<<"FAIL 4-pass scratch\n";return 3;}
   ++count;
  }
 }
 for(size_t k:{size_t(128),size_t(4095),size_t(4096),size_t(4097),size_t(8192),size_t(16383),size_t(16384),size_t(16385),size_t(32768)}){
   const size_t n=300000;
   std::vector<U> keys(k);for(auto& x:keys)x=rng();
   std::vector<U> a(n);
   for(auto&x:a)x=keys[rng()%k];
   if(k>16384){for(size_t i=0;i<k;++i)a[i]=keys[i];}
   auto before=a, expected=a;std::sort(expected.begin(),expected.end());
   bool ok=rune::detail::large_distinct_reconstruct(a.data(),a.size());
   if((ok && a!=expected)||(!ok && a!=before)){
     std::cerr<<"FAIL large freq k="<<k<<"\n";return 4;
   }
   if(k<=16384 && !ok){std::cerr<<"unexpected large freq reject k="<<k<<"\n";return 5;}
   a=before;rune::sort(a);
   if(a!=expected){std::cerr<<"FAIL complete sort large k="<<k<<"\n";return 6;}
   ++count;
 }
 std::vector<U> collisions;
 for(uint64_t x=0;collisions.size()<34 && x<30000000;++x){
   U h=(x^(x>>32))*0x9e3779b97f4a7c15ULL; h^=h>>33;
   if((h>>(64-16))==37)collisions.push_back(x);
 }
 if(collisions.size()!=34){std::cerr<<"missing collision witness\n";return 7;}
 std::vector<U> a(100000);
 for(size_t i=0;i<a.size();++i)a[i]=collisions[i%34];
 auto before=a;
 if(rune::detail::large_distinct_reconstruct(a.data(),a.size()) || a!=before){
   std::cerr<<"collision rejection mutated input\n";return 8;
 }
 ++count;
 std::cout<<"PASSED "<<count<<" v5 targeted boundary/radix checks\n";
}
