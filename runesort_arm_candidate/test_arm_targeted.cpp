#include "rune_sort.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
using U=uint64_t;
static int count=0;
static void run(std::vector<U> a){
 auto correct=a;std::sort(correct.begin(),correct.end());
 rune::sort(a);
 if(a!=correct){std::cerr<<"Differential mismatch case="<<count<<"\n";std::exit(3);}
 ++count;
}
int main(){
 std::mt19937_64 g(20261020);
 U mask=0;for(int b:{0,4,9,13,18,23,27,32,36,41,45,49,53,57,61,63})mask|=U(1)<<b;
 for(size_t n:{size_t(64),size_t(65536),size_t(524287),size_t(524288),size_t(1000000)}){
  for(int r=0;r<4;++r){
    std::vector<U>a(n);
    for(auto& x:a)x=rune::detail::deposit(g()&65535ULL,mask);
    run(a);
    // Create a hidden outlier outside the first 64 elements. It must
    // not be misclassified by an early sample-only assumption.
    if(n>64)a.back()=g();
    run(a);
  }
 }
 for(int r=0;r<16;++r){
  std::vector<U>a(1000000);
  U v=0;for(auto&x:a){v=g();x=v;}
  run(std::move(a));
 }
 std::cout<<"PASS "<<count<<" targeted tests\n";
}
