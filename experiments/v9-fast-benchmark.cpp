#include "runesort_v9/rune_sort.hpp"
#define rune rune_v7
#include "runesort_v7/rune_sort.hpp"
#undef rune
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>
using U=uint64_t;
using clock_t_=std::chrono::steady_clock;
int main(){
  constexpr size_t n=1000000, repeats=4;
  const std::array<U,5> seeds={817103,817109,817111,817127,817129};
  const std::array<int,10> ks={73728,81920,90112,98304,114688,131072,147456,200000,0,-1};
  std::cout<<"keys,n,seed,algorithm,median_ms,correct\n";
  for(int k:ks)for(U seed:seeds){
    std::mt19937_64 rng(seed);std::vector<U> data(n);
    if(k>0){
      std::vector<U> keys(k);for(auto&x:keys)x=rng();
      for(auto&x:data)x=keys[rng()%k];
    }else if(k==0){for(auto&x:data)x=rng();}
    else {for(auto&x:data)x=rng()&65535;}
    auto reference=data;std::sort(reference.begin(),reference.end());
    std::array<std::vector<double>,3> times;
    for(int rep=0;rep<=int(repeats);++rep){
      std::array<int,3> order{0,1,2};std::mt19937_64 random(seed*7+rep*991);std::shuffle(order.begin(),order.end(),random);
      for(int alg:order){
        auto copy=data;auto begin=clock_t_::now();
        if(alg==0)rune::sort(copy);
        if(alg==1)rune_v7::sort(copy);
        if(alg==2)std::sort(copy.begin(),copy.end());
        auto end=clock_t_::now();
        if(copy!=reference){std::cerr<<"Mismatch, k="<<k<<" alg="<<alg<<"\n";return 2;}
        if(rep>0)times[alg].push_back(std::chrono::duration<double,std::milli>(end-begin).count());
      }
    }
    for(int alg=0;alg<3;++alg){
      auto& t=times[alg];std::sort(t.begin(),t.end());
      std::cout<<k<<','<<n<<','<<seed<<','<<(alg==0?"RUNE_v9":alg==1?"RUNE_v7":"std_sort")<<','
        <<(t[1]+t[2])/2<<",true\n";
    }
    std::cout.flush();
  }
}
