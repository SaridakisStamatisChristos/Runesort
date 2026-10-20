#include "runesort_v13/rune_sort.hpp"
#define rune rune_v7
#include "runesort_v7/rune_sort.hpp"
#undef rune
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>
using U=uint64_t;
using Clock=std::chrono::steady_clock;
static std::vector<U> generate(size_t n,const std::string& mode,U seed) {
  std::mt19937_64 g(seed);std::vector<U> a(n);
  if(mode=="uniform64") {for(auto&x:a)x=g();}
  else if(mode=="random32") {for(auto&x:a)x=g()&0xffffffffULL;}
  else if(mode.rfind("sparse",0)==0) {
    const unsigned bits=static_cast<unsigned>(std::stoul(mode.substr(6)));
    U mask=0;for(unsigned i=0;i<bits;++i) mask|=U(1)<<((i*29+7)&63);
    const U fixed=0xF0F0A50A80000000ULL &~mask;
    for(auto&x:a)x=fixed|rune::detail::deposit(g(),mask);
  } else if(mode.rfind("K",0)==0) {
    const size_t k=std::stoull(mode.substr(1));
    std::vector<U> keys(k);for(auto&x:keys)x=g();
    for(auto&x:a)x=keys[g()%k];
  } else std::abort();
  return a;
}
int main(int argc,char**argv) {
  const size_t n=argc>1?std::stoull(argv[1]):1000000;
  constexpr size_t reps=4;
  const std::array<U,5>seeds={190003,190007,190027,190031,190037};
  const std::array<std::string,9>modes={
    "uniform64","random32","sparse16","sparse17","sparse20",
    "sparse22","sparse23","sparse24","K81920"};
  std::cout<<"workload,n,seed,algorithm,median_ms,min_ms,max_ms,correct\n";
  for(size_t mi=0;mi<modes.size();++mi) for(U seed:seeds) {
    auto input=generate(n,modes[mi],seed+mi*211),oracle=input;
    std::sort(oracle.begin(),oracle.end());
    std::array<std::array<double,reps>,2>times{};
    std::mt19937_64 ord(seed*65537+mi);
    for(size_t rep=0;rep<=reps;++rep) {
      std::array<int,2> order{0,1};
      std::shuffle(order.begin(),order.end(),ord);
      for(const int id:order) {
        auto a=input;
        auto t0=Clock::now();
        if(id==0)rune::sort(a);else rune_v7::sort(a);
        auto t1=Clock::now();
        if(a!=oracle){std::cerr<<"INCORRECT mode="<<modes[mi]<<" n="<<n<<" seed="<<seed<<" id="<<id<<"\n";return 3;}
        if(rep)times[id][rep-1]=std::chrono::duration<double,std::milli>(t1-t0).count();
      }
    }
    for(int id=0;id<2;++id) {
      auto ts=times[id];std::sort(ts.begin(),ts.end());
      std::cout<<modes[mi]<<','<<n<<','<<seed<<','<<(id==0?"RUNE_v13":"RUNE_v7")<<','
        <<(ts[1]+ts[2])*0.5<<','<<ts[0]<<','<<ts[3]<<",true\n";
    }
    std::cout.flush();
  }
}
