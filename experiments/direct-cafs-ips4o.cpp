// Reproducible one-thread comparison of RUNE v5 against established sorters.
// External libraries are fetched on CI runners and are NOT committed to Runesort.
// Copyright (c) 2026 Stamatis-Christos Saridakis. All rights reserved.
#include "runesort_v5/rune_sort.hpp"
#include <cafs2.hpp>
#include <ips4o.hpp>
#include <boost/sort/pdqsort/pdqsort.hpp>
#include <boost/sort/spreadsort/spreadsort.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>
using U=std::uint64_t;
using Clock=std::chrono::steady_clock;
static std::vector<U> generate(const std::string& type, size_t n, U seed) {
  std::mt19937_64 rng(seed);
  std::vector<U> data(n);
  if(type=="uniform64") {
    for(auto& x:data)x=rng();
  }else if(type=="random32") {
    for(auto& x:data)x=rng() & 0xffffffffULL;
  }else if(type=="sparse16") {
    const U mask=(1ULL<<0)|(1ULL<<4)|(1ULL<<9)|(1ULL<<13)|
      (1ULL<<18)|(1ULL<<23)|(1ULL<<27)|(1ULL<<32)|
      (1ULL<<36)|(1ULL<<41)|(1ULL<<45)|(1ULL<<49)|
      (1ULL<<53)|(1ULL<<57)|(1ULL<<61)|(1ULL<<63);
    for(auto& x:data)x=rune::detail::deposit(rng() & 65535ULL,mask);
  }else if(type[0]=='K') {
    size_t k=std::stoull(type.substr(1));
    std::vector<U> keys(k);
    for(auto& x:keys)x=rng();
    for(auto& x:data)x=keys[rng()%k];
  }else if(type=="sorted"||type=="rotated"||type=="nearly") {
    for(auto& x:data)x=rng();
    std::sort(data.begin(),data.end());
    if(type=="rotated" && n)std::rotate(data.begin(),data.begin()+n/3,data.end());
    if(type=="nearly" && n)for(size_t i=0;i<n/100;++i)std::swap(data[rng()%n],data[rng()%n]);
  }else if(type=="local") {
    for(auto&x:data)x=rng();
    std::sort(data.begin(),data.end());
    if(n>1)for(size_t i=0;i<n/100;++i){size_t j=rng()%(n-1);std::swap(data[j],data[j+1]);}
  }else {
    throw std::runtime_error("Unknown dataset "+type);
  }
  return data;
}
static void sorter(int id, std::vector<U>& a) {
  switch(id) {
    case 0: rune::sort(a); break;
    // Provide pdqsort as a fallback, so CAFS gets a strong general-data path.
    case 1: cafs2::cafs_sort(a,[](auto& v){boost::sort::pdqsort(v.begin(),v.end());}); break;
    case 2: ips4o::sort(a.begin(),a.end()); break;
    case 3: boost::sort::pdqsort(a.begin(),a.end()); break;
    case 4: boost::sort::spreadsort::spreadsort(a.begin(),a.end()); break;
    case 5: std::sort(a.begin(),a.end()); break;
    default: std::abort();
  }
}
static const std::array<const char*,6> labels={
 "RUNE_v5","CAFS_pdq","IPS4o_seq","pdqsort","spreadsort","std_sort"
};
int main(int argc,char**argv) {
  const size_t n=argc>1?std::stoull(argv[1]):1000000;
  const int repetitions=argc>2?std::stoi(argv[2]):4;
  if (repetitions<2 || n<2) return 2;
  const std::array<U,3> seeds={271828ULL,314159ULL,777777ULL};
  const std::array<std::string,12> modes={
    "uniform64","random32","sparse16","K128","K1024","K4096",
    "K8192","K16384","K32768","local","nearly","rotated"
  };
  std::cout<<"workload,n,seed,algorithm,median_ms,min_ms,max_ms,repetitions,correct\n";
  for (size_t mi=0;mi<modes.size();++mi) {
    const auto& mode=modes[mi];
    for(U seed:seeds) {
      const std::vector<U> input=generate(mode,n,seed+mi*131);
      auto oracle=input;
      std::sort(oracle.begin(),oracle.end());
      std::array<std::vector<double>,6> timings;
      for(int rep=0;rep<=repetitions;++rep) {
        std::array<int,6> order={0,1,2,3,4,5};
        std::mt19937_64 reorder(seed+mi*991+rep*7919);
        std::shuffle(order.begin(),order.end(),reorder);
        for(int id:order) {
          auto a=input; // Copy outside the timed interval.
          auto start=Clock::now();
          sorter(id,a);
          auto end=Clock::now();
          if(a!=oracle) {
            std::cerr<<"INCORRECT: "<<labels[id]<<" n="<<n<<" mode="<<mode<<" seed="<<seed<<"\n";
            return 5;
          }
          if(rep>0)timings[id].push_back(
            std::chrono::duration<double,std::milli>(end-start).count());
        }
      }
      for(int id=0;id<6;++id) {
        auto& t=timings[id];
        std::sort(t.begin(),t.end());
        double median=(t[(t.size()-1)/2]+t[t.size()/2])/2.0;
        std::cout<<mode<<','<<n<<','<<seed<<','<<labels[id]<<','
          <<std::fixed<<std::setprecision(5)<<median<<','
          <<t.front()<<','<<t.back()<<','<<t.size()<<",true\n";
      }
      std::cout.flush();
    }
  }
  return 0;
}
