// RuneSort historical public-API comparison and separately flagged kernel probes.
// Expected same-array exact sorting, independently oracle-checked.
#define rune rune_v5
#include "runesort_v5/rune_sort.hpp"
#undef rune
#define rune rune_v6
#include "runesort_v6/rune_sort.hpp"
#undef rune
#define rune rune_v7
#include "runesort_v7/rune_sort.hpp"
#undef rune
#define rune rune_v13
#include "runesort_v13/rune_sort.hpp"
#undef rune
#define rune rune_v14
#include "runesort_v14/rune_sort.hpp"
#undef rune
#define rune rune_v16
#include "runesort_v16/rune_sort.hpp"
#undef rune
#include "runesort_v26/rune_sort.hpp"
#include "runesort_v24/rune_sort.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <numeric>
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

static constexpr std::size_t K = 15;
static constexpr std::array<const char*,K> labels = {"std_sort","RUNE5_full","RUNE6_full","RUNE7_full","RUNE13_full","RUNE14_full","RUNE16_full","RUNE17_full","RUNE24_full","RUNE25_full","RUNE26b_full","RUNE22_radix11_worker","RUNE22_radix16_worker","RUNE23_prefetch128_worker","RUNE26b_nibble_worker"};
static void sort_id(unsigned id,std::vector<U>& a) {
  switch(id) {
    case 0: std::sort(a.begin(),a.end()); break; // std_sort
    case 1: rune_v5::sort(a); break; // RUNE5_full
    case 2: rune_v6::sort(a); break; // RUNE6_full
    case 3: rune_v7::sort(a); break; // RUNE7_full
    case 4: rune_v13::sort(a); break; // RUNE13_full
    case 5: rune_v14::sort(a); break; // RUNE14_full
    case 6: rune_v16::sort(a); break; // RUNE16_full
    case 7: rune_v17::sort(a); break; // RUNE17_full
    case 8: rune_v24::sort(a); break; // RUNE24_full
    case 9: rune_v25::sort(a); break; // RUNE25_full
    case 10: rune_v26::sort(a); break; // RUNE26b_full
    case 11: rune_v22::radix11(a.data(),a.size()); break; // RUNE22_radix11_worker
    case 12: rune_v22::radix16(a.data(),a.size()); break; // RUNE22_radix16_worker
    case 13: rune_v23::radix16<128>(a.data(),a.size()); break; // RUNE23_prefetch128_worker
    case 14: rune_v26::kernel<true>(a.data(),a.size()); break; // RUNE26b_nibble_worker
    default: std::abort();
  }
}
int main(int argc,char**argv) {
  if(argc!=2){std::cerr<<"Usage: benchmark <1000000|4000000>\n";return 2;}
  const std::size_t n=std::stoull(argv[1]);
  if(n!=1000000 && n!=4000000){std::cerr<<"unsupported n\n";return 2;}
  constexpr unsigned Reps=3;
  const std::array<U,5> seeds{20301001,20301011,20301021,20301031,20301041};
  const std::array<std::string,10> modes{"uniform64","random32","K85000","sorted","alternating","reverse","sparse16","almostsorted","sortedprefix","entropy_prefix"};
  std::cout<<"mode,n,seed,algorithm,median_ms,min_ms,max_ms,trial0_ms,trial1_ms,trial2_ms,correct,simd_available\n";
  for(std::size_t mi=0;mi<modes.size();++mi)for(const U seed:seeds){
    auto source=generate(n,modes[mi],seed+mi*229),oracle=source;
    std::sort(oracle.begin(),oracle.end()); // never timed
    std::array<std::array<double,Reps>,K> times{};
    std::mt19937_64 order(seed+mi*447+uint64_t(n));
    for(unsigned rep=0;rep<=Reps;++rep){
      std::array<unsigned,K> ids{};
      std::iota(ids.begin(),ids.end(),0u);
      std::shuffle(ids.begin(),ids.end(),order);
      for(unsigned id:ids){
        auto a=source; // copy outside the timed section
        const auto start=Clock::now();
        sort_id(id,a);
        const auto finish=Clock::now();
        if(a!=oracle){
          std::cerr<<"INCORRECT "<<labels[id]<<" "<<modes[mi]<<" n="<<n<<" seed="<<seed<<"\n";
          return 4;
        }
        if(rep)times[id][rep-1]=std::chrono::duration<double,std::milli>(finish-start).count();
      }
    }
    for(unsigned id=0;id<K;++id){
      auto t=times[id];std::sort(t.begin(),t.end());
      std::cout<<modes[mi]<<','<<n<<','<<seed<<','<<labels[id]<<','
        <<std::setprecision(17)<<t[1]<<','<<t.front()<<','<<t.back()<<','
        <<times[id][0]<<','<<times[id][1]<<','<<times[id][2]
        <<",true,"<<rune_v25::simd_available()<<'\n';
    }
    std::cout.flush();
  }
}
