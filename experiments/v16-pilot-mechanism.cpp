#include "runesort_v16/rune_sort.hpp"
#define rune rune_v14
#include "runesort_v14/rune_sort.hpp"
#undef rune
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>
using U=uint64_t;
using Clock=std::chrono::steady_clock;
int main(int argc,char**argv){
 size_t n=argc>1?std::stoull(argv[1]):4000000;
 const std::array<size_t,9>K={16384,65536,73728,81920,85000,90000,95000,100000,131072};
 const std::array<U,4>seeds={16001,16009,16033,16061};
 std::cout<<"workload,n,seed,old_pilot,new_pilot,algorithm,median_ms,correct\n";
 for(size_t k:K)for(U seed:seeds){
  std::mt19937_64 gen(seed*91+k);
  std::vector<U>keys(k),base(n);
  for(auto &x:keys)x=gen();
  for(auto &x:base)x=keys[gen()%k];
  const auto oracle=[&](){auto v=base;std::sort(v.begin(),v.end());return v;}();
  const bool old=rune_v14::detail::fifth_tier_pilot(base.data(),n);
  const bool now=rune::detail::fifth_tier_pilot(base.data(),n);
  std::array<std::array<double,3>,2>times{};
  for(int rep=0;rep<=3;++rep){
   std::array<int,2>order{0,1};
   std::mt19937_64 scramble(seed+rep*7919);
   std::shuffle(order.begin(),order.end(),scramble);
   for(int id:order){
    auto a=base;const auto st=Clock::now();
    if(id==0)rune_v14::sort(a);else rune::sort(a);
    auto et=Clock::now();
    if(a!=oracle){std::cerr<<"INVALID K"<<k<<" "<<id<<"\n";return 5;}
    if(rep)times[id][rep-1]=std::chrono::duration<double,std::milli>(et-st).count();
   }
  }
  for(int id=0;id<2;++id){
   auto x=times[id];std::sort(x.begin(),x.end());
   std::cout<<"K"<<k<<','<<n<<','<<seed<<','<<(old?1:0)<<','<<(now?1:0)
   <<','<<(id?"v16":"v14")<<','<<std::fixed<<std::setprecision(6)<<x[1]<<",true\n";
  }
  std::cout.flush();
 }
}
