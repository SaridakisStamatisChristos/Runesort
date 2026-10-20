#include "rune_sort.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>
#include <limits>
#include <cstdlib>
using U=uint64_t;
static uint64_t cases=0;
void verify(std::vector<U> a){auto e=a;std::sort(e.begin(),e.end());rune::sort(a);if(a!=e){std::cerr<<"FAIL sort case "<<cases<<" n="<<a.size()<<"\n";std::exit(3);}++cases;}
void verify_extended(std::vector<U> a){auto e=a;auto before=a;std::sort(e.begin(),e.end());bool ok=rune::detail::extended_distinct_reconstruct(a.data(),a.size());if((ok&&a!=e)||(!ok&&a!=before)){std::cerr<<"FAIL bounded reconstruction case "<<cases<<"\n";std::exit(4);}++cases;}
int main(){
 std::mt19937_64 rng(0x52756e65abcdULL);
 for(size_t n:{size_t(63),size_t(64),size_t(65),size_t(4096),size_t(65535),size_t(65536),size_t(65537),size_t(100000)}){
  for(size_t k:{size_t(1),size_t(2),size_t(1023),size_t(1024),size_t(1025),size_t(2047),size_t(2048),size_t(4095),size_t(4096),size_t(4097),size_t(8192)}){
   std::vector<U> keys(k);for(auto&x:keys)x=rng();
   for(int form=0;form<3;++form){std::vector<U>a(n);
    for(auto&x:a)x=keys[rng()%k];
    if(form==1){for(size_t i=0;i<std::min(n,k);++i)a[i]=keys[i];}
    if(form==2){for(size_t i=0;i<n;i+=64)a[i]=rng();}
    verify(a);
    if(n>=65536)verify_extended(std::move(a));
   }
  }
 }
 for(int trial=0;trial<500;++trial){size_t n=65+(rng()%15000);std::vector<U>a(n);
   int mode=trial%8;
   for(auto&x:a){x=(mode==0?rng():mode==1?rng()&0xffffffffULL:mode==2?rng()&65535:mode==3?rng()&255:mode==4?~U(0):mode==5?U(0):mode==6?U(1)<< (rng()%64):rng());}
   if(trial%4==0)std::sort(a.begin(),a.end());
   if(trial%4==1){std::sort(a.begin(),a.end());std::rotate(a.begin(),a.begin()+n/3,a.end());}
   verify(std::move(a));
 }
 for(size_t n:{size_t(65),size_t(4096),size_t(65536),size_t(100000)}){
   std::vector<U>a(n);
   for(size_t i=0;i<n;++i)a[i]=rng()&0xffffffffULL;
   if(n>64)a[n-1]|=U(1)<<60;
   verify(a);
   for(auto&x:a)x|=0xaabbccdd00000000ULL;
   verify(std::move(a));
 }
 std::vector<U> collisions;
 for(uint64_t x=0;collisions.size()<34 && x<3000000;++x){
  U h=(x^(x>>32))*0x9e3779b97f4a7c15ULL;h^=h>>33;
  if((h>>(64-14))==19)collisions.push_back(x);
 }
 if(collisions.size()<34){std::cerr<<"Could not construct collision set\n";return 5;}
 std::vector<U> a;for(size_t i=0;i<1024;++i)a.push_back(collisions[i%collisions.size()]);
 verify_extended(a);verify(a);
 std::cout<<"PASSED "<<cases<<" v4 targeted checks\n";
}
