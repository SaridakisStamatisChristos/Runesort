// PINNED: external comparator sources fetched by CI; RUNE code remains proprietary.
#include "runesort_v15/rune_sort.hpp"
#define rune rune_v14
#include "runesort_v14/rune_sort.hpp"
#undef rune
#define rune rune_v13
#include "runesort_v13/rune_sort.hpp"
#undef rune
#define rune rune_arm16
#include "experiments/v14_baselines/arm16/rune_sort.hpp"
#undef rune
#include <ips4o.hpp>
#include <hwy/contrib/sort/vqsort.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>
using U=std::uint64_t;
using Clock=std::chrono::steady_clock;
static U mask(const std::string&s,std::mt19937_64&g){
 U m=0;
 if(s=="spread16")for(unsigned i=0;i<16;++i)m|=U(1)<<((i*29+7)&63);
 else if(s=="random16")while(rune::detail::popcount(m)<16)m|=U(1)<<(g()%64);
 else if(s=="contig16mid")m=((U(1)<<16)-1)<<22;
 else if(s=="split16")m=U(255)|(U(255)<<48);
 else if(s=="fourbytes16")m=U(15)|(U(15)<<16)|(U(15)<<32)|(U(15)<<48);
 else if(s=="sparse20")for(unsigned i=0;i<20;++i)m|=U(1)<<((i*29+7)&63);
 else if(s=="sparse23")for(unsigned i=0;i<23;++i)m|=U(1)<<((i*29+7)&63);
 return m;
}
static std::vector<U> generate(size_t n,const std::string&s,U seed){
 std::mt19937_64 g(seed);std::vector<U>a(n);
 if(s=="uniform64"){for(auto&x:a)x=g();return a;}
 if(s=="K65536"){std::vector<U>keys(65536);for(auto&x:keys)x=g();
  for(auto&x:a)x=keys[g()%keys.size()];return a;}
 U m=mask(s,g);
 if(!m)std::abort();
 U fixed=g()&~m;
 for(auto&x:a)x=fixed|rune::detail::deposit(g(),m);
 return a;
}
static void sorter(int id,std::vector<U>& a){
 switch(id){
 case 0:rune::sort(a);break;
 case 1:rune_v13::sort(a);break;
 case 2:rune_arm16::sort(a);break;
 case 3:hwy::VQSort(a.data(),a.size(),hwy::SortAscending{});break;
 case 4:ips4o::sort(a.begin(),a.end());break;
 case 5:rune_v14::sort(a);break;
 default:std::abort();
 }
}
int main(int argc,char**argv){
 const size_t n=argc>1?std::stoull(argv[1]):1000000;
 constexpr int reps=4;
 const std::array<U,5> seeds={512003,512009,512011,512021,512047};
 const std::array<std::string,9>modes={"spread16","random16","contig16mid",
 "split16","fourbytes16","sparse20","sparse23","uniform64","K65536"};
 const std::array<const char*,6>labels={"RUNE_v15","RUNE_v13","RUNE_arm16","VQSort","IPS4o_seq","RUNE_v14"};
 std::cout<<"workload,n,seed,algorithm,median_ms,min_ms,max_ms,correct\n";
 for(size_t mi=0;mi<modes.size();++mi)for(U seed:seeds){
  auto a0=generate(n,modes[mi],seed+mi*389),oracle=a0;
  std::sort(oracle.begin(),oracle.end());
  std::array<std::array<double,reps>,6>times{};
  std::mt19937_64 orderrng(seed+mi*777);
  for(int rep=0;rep<=reps;++rep){
   std::array<int,6>order{0,1,2,3,4,5};
   std::shuffle(order.begin(),order.end(),orderrng);
   for(int id:order){
    auto a=a0;auto st=Clock::now();
    sorter(id,a);
    auto en=Clock::now();
    if(a!=oracle){std::cerr<<"INCORRECT "<<labels[id]<<" "<<modes[mi]<<" "<<n<<"\n";return 3;}
    if(rep)times[id][rep-1]=std::chrono::duration<double,std::milli>(en-st).count();
   }
  }
  for(int id=0;id<6;++id){
    auto z=times[id];std::sort(z.begin(),z.end());
    std::cout<<modes[mi]<<','<<n<<','<<seed<<','<<labels[id]<<','
      <<(z[1]+z[2])*0.5<<','<<z.front()<<','<<z.back()<<",true\n";
  }
  std::cout.flush();
 }
}
