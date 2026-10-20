// ARM64 sparse-bit dispatch ablation: same-source frozen v7 against candidate.
// Copyright (c) 2026 Stamatis-Christos Saridakis. All rights reserved.
#include "runesort_arm_candidate/rune_sort.hpp"
#define rune rune_v7
#include "runesort_v7/rune_sort.hpp"
#undef rune
#include <boost/sort/pdqsort/pdqsort.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>
using U=std::uint64_t;
using clock_type=std::chrono::steady_clock;
std::vector<U> make(const std::string& mode,size_t n,U seed) {
 std::mt19937_64 g(seed);std::vector<U>a(n);
 if(mode=="uniform64"){for(auto& x:a)x=g();return a;}
 if(mode=="random32"){for(auto& x:a)x=g()&0xffffffffULL;return a;}
 if(mode=="K32768"||mode=="K65536"){
   size_t k=mode=="K32768"?32768:65536;
   std::vector<U>keys(k);for(auto&x:keys)x=g();
   for(auto&x:a)x=keys[g()%k];return a;
 }
 U mask=0;
 const std::array<int,16> bits={0,4,9,13,18,23,27,32,36,41,45,49,53,57,61,63};
 unsigned varied=mode=="sparse16"||mode=="outlier16"?16:mode=="sparse15"?15:12;
 for(size_t i=0;i<varied;++i)mask|=U(1)<<bits[i];
 for(auto&x:a)x=rune::detail::deposit(g(),mask);
 if(mode=="outlier16" && n>64)a[n-1]=g();
 return a;
}
static const std::array<const char*,3> labels={"candidate","frozen_v7","pdqsort"};
int main(int argc,char** argv) {
 const size_t n=argc>1?std::stoull(argv[1]):1000000;
 const int reps=argc>2?std::stoi(argv[2]):4;
 const std::array<U,4> seeds={180007,180017,180023,180031};
 const std::array<std::string,9> modes={"sparse16","sparse15","sparse12","outlier16","uniform64","random32","K32768","K65536","outlier16"};
 std::cout<<"mode,n,seed,algo,median_ms,min_ms,max_ms,repetitions,correct\n";
 for(size_t mi=0;mi<modes.size();++mi)for(auto seed:seeds){
  auto original=make(modes[mi],n,seed+131*mi);
  auto reference=original;std::sort(reference.begin(),reference.end());
  std::array<std::vector<double>,3> times;
  for(int rep=0;rep<=reps;++rep){
   std::array<int,3> order={0,1,2};
   std::mt19937_64 perm(seed+7919*rep+mi*113);
   std::shuffle(order.begin(),order.end(),perm);
   for(int algo:order){
    auto a=original;
    auto start=clock_type::now();
    if(algo==0)rune::sort(a);
    if(algo==1)rune_v7::sort(a);
    if(algo==2)boost::sort::pdqsort(a.begin(),a.end());
    auto end=clock_type::now();
    if(a!=reference){std::cerr<<"FAILED oracle "<<modes[mi]<<" "<<labels[algo]<<"\n";return 3;}
    if(rep)times[algo].push_back(std::chrono::duration<double,std::milli>(end-start).count());
   }
  }
  for(int algo=0;algo<3;++algo){
   auto&x=times[algo];std::sort(x.begin(),x.end());
   auto med=(x[(x.size()-1)/2]+x[x.size()/2])/2;
   std::cout<<modes[mi]<<','<<n<<','<<seed<<','<<labels[algo]<<','
            <<std::fixed<<std::setprecision(5)<<med<<','
            <<x.front()<<','<<x.back()<<','<<reps<<",true\n";
  }
  std::cout.flush();
 }
}
