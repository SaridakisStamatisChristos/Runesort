#include "rune_sort.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>
#include <cstdlib>
using U=uint64_t;
static size_t cases=0;
void verify(std::vector<U> a) { auto e=a;std::sort(e.begin(),e.end());rune::sort(a); if(a!=e){std::cerr<<"FAIL #"<<cases<<" length="<<a.size()<<"\n";std::exit(2);} ++cases;}
int main() {
 std::mt19937_64 rng(0x52756e65ULL);
 for(size_t n : {64u,65u,127u,511u,4095u,4096u,65535u,65536u,65537u,100000u}) {
   for(size_t k : {2u,16u,63u,64u,127u,128u,129u,256u,512u,1024u,2048u,2049u,4096u}){
     std::vector<U> keys(k);for(auto& v:keys)v=rng();
     for(int t=0;t<3;++t){
       std::vector<U> a(n);
       for(auto& x:a)x=keys[rng()%k];
       if(t==1)for(size_t i=0;i<std::min(size_t(64),n);++i)a[i]=keys[i%k];
       if(t==2)for(size_t i=64;i<n;i+=64)a[i]=rng();
       verify(std::move(a));
     }
   }
   for(unsigned limit : {1u,2u,4u,8u,16u}){
     std::vector<U>a(n);std::iota(a.begin(),a.end(),U(0));
     for(size_t i=0;i<n/100;++i){size_t j=rng()%(n-1);std::swap(a[j],a[j+1]);}
     verify(a);
     for(size_t i=0;i<n/20;++i){size_t j=rng()%n;size_t swap_with=std::min(j+size_t(limit),n-1);std::swap(a[j],a[swap_with]);}
     verify(a);
   }
 }
 for(int trial=0;trial<120;++trial){
   std::vector<U>a(70000);for(auto &x:a)x=rng()%8192;
   auto original=a;
   bool accepted=rune::detail::medium_distinct_reconstruct(a.data(),a.size());
   auto expected=original;std::sort(expected.begin(),expected.end());
   if(!accepted && a!=original){std::cerr<<"Failed reconstruction modified input\n";return 3;}
   if(accepted && a!=expected){std::cerr<<"Reconstruction wrong\n";return 4;}
   verify(std::move(original));
 }
 for(int trial=0;trial<1000;++trial){size_t n=65+(rng()%8192);std::vector<U> a(n);
   for(size_t i=0;i<n;++i)a[i]=rng();std::sort(a.begin(),a.end());
   for(size_t t=0;t<n/10;++t){size_t i=rng()%(n-1);std::swap(a[i],a[i+1]);}
   verify(a);
 }
 std::cout<<"PASSED "<<cases<<" v3 targeted differential checks\n";
}
