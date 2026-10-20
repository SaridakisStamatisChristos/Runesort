// Experimental 131K-key exact reconstruction; independent of frozen RUNE v7.
// Copyright (c) 2026 Stamatis-Christos Saridakis. All rights reserved.
#include "runesort_v7/rune_sort.hpp"
#include <boost/sort/pdqsort/pdqsort.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>
using U=std::uint64_t;
using Clock=std::chrono::steady_clock;
template<size_t Slots,unsigned Shift>
bool frequency(std::vector<U>& a) {
  constexpr size_t Max=131072;
  std::vector<U> keys(Slots);
  std::vector<uint32_t> counts(Slots);
  std::vector<uint8_t> used(Slots);
  size_t unique=0;
  for(U x:a){
    U h=(x^(x>>32))*0x9e3779b97f4a7c15ULL;h^=h>>33;
    size_t b=size_t(h>>Shift);
    unsigned probes=0;
    while(used[b]&&keys[b]!=x) {
      if(++probes>=32)return false;
      b=(b+1)&(Slots-1);
    }
    if(!used[b]) {
      if(unique==Max)return false;
      used[b]=1;keys[b]=x;++unique;
    }
    if(counts[b]==UINT32_MAX)return false;
    ++counts[b];
  }
  std::vector<std::pair<U,uint32_t>> pairs;
  pairs.reserve(unique);
  for(size_t i=0;i<Slots;++i)if(used[i])pairs.emplace_back(keys[i],counts[i]);
  std::sort(pairs.begin(),pairs.end(),[](const auto& x,const auto& y){return x.first<y.first;});
  size_t pos=0;
  for(const auto& pair:pairs){
    std::fill_n(a.data()+pos,pair.second,pair.first);pos+=pair.second;
  }
  return pos==a.size();
}
std::vector<U> dataset(const std::string& name,size_t n,U seed) {
  std::mt19937_64 rng(seed);
  std::vector<U> a(n);
  if(name=="random64")for(auto&x:a)x=rng();
  else if(name=="sparse16"){
    U mask=0;for(int bit:{0,4,9,13,18,23,27,32,36,41,45,49,53,57,61,63})mask|=U(1)<<bit;
    for(auto&x:a)x=rune::detail::deposit(rng()&65535,mask);
  }else{
    size_t k=std::stoull(name.substr(1));
    std::vector<U> keys(k);
    for(auto&x:keys)x=rng();
    for(auto&x:a)x=keys[rng()%k];
  }
  return a;
}
static bool run(int algo,std::vector<U>& a){
  if(algo==0){rune::sort(a);return false;}
  if(algo==1){bool ok=frequency<262144,46>(a);if(!ok)rune::sort(a);return ok;}
  if(algo==2){bool ok=frequency<524288,45>(a);if(!ok)rune::sort(a);return ok;}
  boost::sort::pdqsort(a.begin(),a.end());return false;
}
int main(int argc,char** argv){
  size_t n=argc>1?std::stoull(argv[1]):1000000;
  int reps=argc>2?std::stoi(argv[2]):3;
  if(n<2||reps<2)return 2;
  std::vector<U> rejection(200000);for(size_t i=0;i<rejection.size();++i)rejection[i]=i;
  auto original=rejection;
  if(frequency<262144,46>(rejection)||rejection!=original||frequency<524288,45>(rejection)||rejection!=original){
    std::cerr<<"Failed non-mutating rejection invariant\n";return 3;
  }
  const std::array<U,4> seeds={130031,130037,130043,130051};
  const std::array<std::string,13> modes={"K32768","K49152","K65536","K73728","K81920","K90112","K98304","K114688","K131072","K147456","K180224","random64","sparse16"};
  const std::array<const char*,4> labels={"frozen_v7","freq131k_256k","freq131k_512k","pdqsort"};
  std::cout<<"mode,n,seed,algorithm,median_ms,min_ms,max_ms,accepted_trials,repetitions,correct\n";
  for(size_t m=0;m<modes.size();++m)for(U seed:seeds){
    auto original=dataset(modes[m],n,seed+m*131);
    auto expected=original;std::sort(expected.begin(),expected.end());
    std::array<std::vector<double>,4> times;
    std::array<int,4> accepted{};
    for(int rep=0;rep<=reps;++rep){
      std::array<int,4> order={0,1,2,3};
      std::mt19937_64 rnd(seed+m*991+rep*7919);
      std::shuffle(order.begin(),order.end(),rnd);
      for(int algo:order){
        auto a=original;
        auto start=Clock::now();
        bool ok=run(algo,a);
        auto stop=Clock::now();
        if(a!=expected){std::cerr<<"Incorrect "<<modes[m]<<" "<<labels[algo]<<"\n";return 4;}
        if(rep){times[algo].push_back(std::chrono::duration<double,std::milli>(stop-start).count());accepted[algo]+=ok;}
      }
    }
    for(int algo=0;algo<4;++algo){
      auto& t=times[algo];std::sort(t.begin(),t.end());
      double median=(t[(t.size()-1)/2]+t[t.size()/2])/2.0;
      std::cout<<modes[m]<<','<<n<<','<<seed<<','<<labels[algo]<<','
       <<std::fixed<<std::setprecision(5)<<median<<','<<t.front()<<','<<t.back()
       <<','<<accepted[algo]<<','<<reps<<",true\n";
    }
    std::cout.flush();
  }
}
