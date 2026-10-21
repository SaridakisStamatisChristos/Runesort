// Full public-dispatch competition; upstream revisions pinned from current default heads on 2026-10-21.
#include "runesort_v26/rune_sort.hpp"
#include "runesort_v24/rune_sort.hpp"
#ifndef RUNE26_LOCAL_ONLY
#include <hwy/contrib/sort/vqsort.h>
#include <ips4o.hpp>
#endif
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>
using U=std::uint64_t;using Clock=std::chrono::steady_clock;
static std::vector<U> generate(std::size_t n,const std::string& mode,U seed){
  std::mt19937_64 g(seed);std::vector<U> a(n);
  if(mode=="uniform64")for(auto&x:a)x=g();
  else if(mode=="random32")for(auto&x:a)x=g()&UINT64_C(0xffffffff);
  else if(mode=="K85000"){
    std::vector<U> keys(85000);for(auto&x:keys)x=g();
    for(auto&x:a)x=keys[g()%keys.size()];
  }else if(mode=="sorted"){
    for(auto&x:a)x=g();std::sort(a.begin(),a.end());
  }else if(mode=="alternating"){
    for(std::size_t i=0;i<n;++i)a[i]=(i&1)?0:UINT64_MAX;
  }else if(mode=="reverse"){
    for(auto&x:a)x=g();std::sort(a.rbegin(),a.rend());
  }else if(mode=="sparse16"){
    for(auto&x:a)x=(g()&65535)<<48;
  }else if(mode=="almostsorted"){
    for(std::size_t i=0;i<n;++i)a[i]=i;
    for(std::size_t i=0;i<n/1000;++i)std::swap(a[g()%n],a[g()%n]);
  }else if(mode=="sortedprefix"){
    for(auto&x:a)x=g();std::sort(a.begin(),a.begin()+std::min(n,std::size_t(64)));
  }else if(mode=="entropy_prefix"){
    for(std::size_t i=0;i<n;++i)a[i]=i;
    for(std::size_t i=0;i<std::min(n,std::size_t(64));++i)a[i]=g();
  }else std::abort();
  return a;
}
static void sort_id(unsigned id,std::vector<U>& a){
  switch(id){
    case 0:rune_v17::sort(a);break;
    case 1:rune_v25::sort(a);break;
    case 2:rune_v26::sort(a);break;
    case 3:rune_v26::ablation(a.data(),a.size());break;
    case 4:
#ifndef RUNE26_LOCAL_ONLY
      hwy::VQSort(a.data(),a.size(),hwy::SortAscending{});
#else
      std::sort(a.begin(),a.end());
#endif
      break;
    case 5:
#ifndef RUNE26_LOCAL_ONLY
      ips4o::sort(a.begin(),a.end());
#else
      std::sort(a.begin(),a.end());
#endif
      break;
    default:std::abort();
  }
}
int main(int argc,char**argv){
  const std::size_t n=argc>1?std::stoull(argv[1]):1000000;
  const bool pilot=argc>2 && std::string(argv[2])=="pilot";
  constexpr unsigned Reps=3;
  const std::vector<U> seeds=pilot ? std::vector<U>{991009} :
    std::vector<U>{20301001,20301011,20301021,20301031,20301041};
  const std::array<std::string,10> modes{"uniform64","random32","K85000","sorted","alternating",
    "reverse","sparse16","almostsorted","sortedprefix","entropy_prefix"};
  std::array<const char*,6> labels{
    "RUNE17_full","RUNE25_full_network","RUNE26_full_nibble","RUNE26_full_stdleaf","VQSort_current","IPS4o_seq_current"};
#ifdef RUNE26_LOCAL_ONLY
  labels[4]="std_sort_control_A";labels[5]="std_sort_control_B";
#endif
  std::cout<<"mode,n,seed,algorithm,median_ms,min_ms,max_ms,trial0_ms,trial1_ms,trial2_ms,correct,simd_available\n";
  for(std::size_t mi=0;mi<modes.size();++mi)for(const U seed:seeds){
    if(pilot && (mi!=0 || seed!=seeds[0]))continue;
    auto source=generate(n,modes[mi],seed+mi*229),oracle=source;
    std::sort(oracle.begin(),oracle.end());
    std::array<std::array<double,Reps>,6> times{};
    std::mt19937_64 order(seed+mi*447);
    for(unsigned rep=0;rep<=Reps;++rep){
      std::array<unsigned,6> ids{0,1,2,3,4,5};
      std::shuffle(ids.begin(),ids.end(),order);
      for(unsigned id:ids){
        auto a=source;
        const auto start=Clock::now();sort_id(id,a);
        const auto finish=Clock::now();
        if(a!=oracle){
          std::cerr<<"INCORRECT "<<labels[id]<<" "<<modes[mi]<<" n="<<n<<" seed="<<seed<<"\n";
          return 4;
        }
        if(rep)times[id][rep-1]=std::chrono::duration<double,std::milli>(finish-start).count();
      }
    }
    for(unsigned id=0;id<6;++id){
      auto t=times[id];std::sort(t.begin(),t.end());
      std::cout<<modes[mi]<<','<<n<<','<<seed<<','<<labels[id]<<','
        <<std::setprecision(17)<<t[1]<<','<<t.front()<<','<<t.back()<<','<<times[id][0]<<','<<times[id][1]<<','<<times[id][2]<<",true,"<<rune_v25::simd_available()<<'\n';
    }
    std::cout.flush();
  }
}
