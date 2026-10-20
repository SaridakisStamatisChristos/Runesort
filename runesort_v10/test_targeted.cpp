#include "rune_sort.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
using U=uint64_t;
static size_t checks=0;
static void validate(std::vector<U> a,bool direct) {
  auto reference=a;std::sort(reference.begin(),reference.end());
  if(direct){auto b=a;rune::detail::radix10_full(b.data(),b.size());
    if(b!=reference){std::cerr<<"v10 exact radix failure\n";std::exit(3);}
  }
  rune::sort(a);
  if(a!=reference){std::cerr<<"v10 dispatcher failure #"<<checks<<"\n";std::exit(4);}
  ++checks;
}
int main(){
 std::mt19937_64 rng(0x20261020ULL);
 for(size_t n:{size_t(0),size_t(1),size_t(2),size_t(63),size_t(64),
                size_t(65535),size_t(65536),size_t(524287),
                size_t(524288),size_t(1000000)}){
   for(size_t k:{size_t(16),size_t(40960),size_t(73728),
                  size_t(81920),size_t(90112),size_t(114688),
                  size_t(131072),size_t(147456),size_t(200000)}){
     std::vector<U> choices(k),a(n);
     for(auto&x:choices)x=rng();
     for(auto&x:a)x=choices[rng()%k];
     validate(a,n<=65536 || n>=524288);
   }
   std::vector<U> a(n);
   for(auto&x:a)x=rng();validate(a,n<=65536 || n>=524288);
   for(auto&x:a)x=rng()&0xffffffffULL;validate(a,n<=65536);
   std::sort(a.begin(),a.end());validate(a,false);
   if(n)std::rotate(a.begin(),a.begin()+n/3,a.end());
   validate(a,false);
 }
 // Failure of cardinality predictions can change only performance.
 for(int i=0;i<150;++i){
   std::vector<U> a(16384);
   for(auto&x:a)x=rng();
   if(i&1)for(auto&x:a)x&=65535ULL;
   validate(a,true);
 }
 std::cout<<"PASSED "<<checks<<" targeted v10 correctness checks\n";
}
