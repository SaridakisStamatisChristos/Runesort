#define RUNE27_TRACE_PATH
// RUNE v27 qualification: single-thread unsigned uint64 ascending sorter.
 // Baselines remain under their respective licenses and are fetched by CI.
#include "runesort_v27/rune_sort.hpp"
#include "runesort_v27/byte_msd.hpp"
#include "runesort_v24/rune_sort.hpp"
#if defined(__x86_64__) && defined(__AVX2__)
#include <x86simdsort-static-incl.h>
#define RUNE27_XSS 1
#else
#define RUNE27_XSS 0
#endif
#include <hwy/contrib/sort/vqsort.h>
#include <ips4o.hpp>
#include <boost/sort/pdqsort/pdqsort.hpp>
#include <boost/sort/spreadsort/spreadsort.hpp>
#if defined(__x86_64__) && defined(__AVX2__) && (defined(__BMI__) || defined(__BMI2__))
#define RUNE_HAS_CAFS 1
#include <cafs2.hpp>
#else
#define RUNE_HAS_CAFS 0
#endif

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>
using U=std::uint64_t;
using Clock=std::chrono::steady_clock;
using A=std::vector<U>;
struct Implementation{
 const char* name;
 void (*run)(A&);
};

static U sparse_mask(unsigned bits,bool alternate=false) {
 U m=0;
 // Both multipliers are odd modulo 64, hence 16/17/20/23/24
 // selected bit locations are distinct.
 for(unsigned i=0;i<bits;++i)m|=U(1)<<(((alternate?19:29)*i+7)&63);
 return m;
}
static std::vector<U> generate(const std::string& mode,size_t n,U seed){
 std::mt19937_64 rng(seed);
 A a(n);
 if(mode=="uniform64"){
  for(auto&x:a)x=rng();
 }else if(mode=="random32"){
  for(auto&x:a)x=rng()&0xffffffffULL;
 }else if(mode=="random16"){
  for(auto&x:a)x=rng()&0xffffULL;
 }else if(mode=="extremes"){
  for(size_t i=0;i<n;++i)a[i]=(i&1)?(std::numeric_limits<U>::max()-U(rng()&4095)):U(rng()&4095);
 }else if(mode.rfind("sparse",0)==0){
  // sparse16, sparse16alt, sparse17, sparse20, sparse23, sparse24
  const bool alt=mode=="sparse16alt";
  const unsigned bits=alt?16u:static_cast<unsigned>(std::stoul(mode.substr(6)));
  const U mask=sparse_mask(bits,alt),constant=rng()&~mask;
  for(auto&x:a)x=constant|rune_v17::detail::deposit(rng(),mask);
 }else if(mode=="contig16"){
  const U mask=U(0xffff)<<32,constant=rng()&~mask;
  for(auto&x:a)x=constant|rune_v17::detail::deposit(rng(),mask);
 }else if(mode=="split16"){
  const U mask=U(0xff)|(U(0xff)<<48),constant=rng()&~mask;
  for(auto&x:a)x=constant|rune_v17::detail::deposit(rng(),mask);
 }else if(!mode.empty()&&mode[0]=='K'){
  const size_t k=std::stoull(mode.substr(1));
  if(k==0||k>n*2)throw std::invalid_argument("unsupported cardinality");
  std::vector<U>keys(k);
  for(auto&x:keys)x=rng();
  for(auto&x:a)x=keys[rng()%k];
 }else if(mode=="equal"){
  std::fill(a.begin(),a.end(),rng());
 }else if(mode=="sorted"||mode=="reversed"||mode=="rotated"||mode=="nearly"||mode=="local"){
  for(auto&x:a)x=rng();
  std::sort(a.begin(),a.end());
  if(mode=="reversed")std::reverse(a.begin(),a.end());
  if(mode=="rotated"&&n)std::rotate(a.begin(),a.begin()+n/3,a.end());
  if(mode=="nearly"&&n)for(size_t i=0;i<n/100;++i)std::swap(a[rng()%n],a[rng()%n]);
  if(mode=="local"&&n>1)for(size_t i=0;i<n/100;++i){size_t j=rng()%(n-1);std::swap(a[j],a[j+1]);}
 }else if(mode=="organpipe"){
  for(size_t i=0;i<n;++i)a[i]=U(std::min(i,n-1-i));
 }else if(mode=="sawtooth"){
  for(size_t i=0;i<n;++i)a[i]=U(i%1024);
 }else if(mode=="alternating"){
  for(size_t i=0;i<n;++i)a[i]=(i&1)?U(i):std::numeric_limits<U>::max()-U(i);
 }else throw std::invalid_argument("unknown workload "+mode);
 return a;
}
static std::vector<Implementation> implementations(){return {
 {"RUNE_v27",[](A&v){rune_v27::sort(v);}},
 {"RUNE_v17",[](A&v){rune_v17::sort(v);}},
 {"VQSort",[](A&v){hwy::VQSort(v.data(),v.size(),hwy::SortAscending{});}},
 {"x86_simd_sort",[](A&v){x86simdsortStatic::qsort(v.data(),v.size());}}
};}
static std::vector<std::string> modes(const std::string& group){
 if(group=="entropy")return {"sparse17","sparse20","random16","uniform64"};
 if(group=="cardinality")return {"K16","K128","K1024","K4096","K8192",
  "K16384","K32768","K40960","K65536","K73728","K81920","K131072"};
 if(group=="ordered")return {"sorted","reversed","rotated","nearly",
  "local","equal","organpipe","sawtooth","alternating"};
 throw std::invalid_argument("unknown group "+group);
}
int main(int argc,char**argv){
 try{
  if(argc<3||argc>5){std::cerr<<"usage: benchmark <group> <n> [reps=5] [seeds=5]\n";return 2;}
  const std::string group=argv[1];
  const size_t n=std::stoull(argv[2]);
  const int reps=argc>3?std::stoi(argv[3]):5;
  const int seeds=argc>4?std::stoi(argv[4]):5;
  if(n<2||reps!=5||seeds!=5)throw std::invalid_argument("requires n>=2, reps=5, seeds=5");
  const std::array<U,5> fixed_seeds={704003,704009,704017,704023,704029};
  auto algos=implementations();
  const auto cases=modes(group);
  std::cout<<"group,workload,n,seed,algorithm,median_ms,min_ms,max_ms,repetitions,correct,trial0_ms,trial1_ms,trial2_ms,trial3_ms,trial4_ms,simd_available,bmi2_available,compact_selected\n";
  for(size_t mi=0;mi<cases.size();++mi){
   for(int si=0;si<seeds;++si){
    const U seed=fixed_seeds[si]+U(mi)*131071;
    A input=generate(cases[mi],n,seed),oracle=input;
    std::sort(oracle.begin(),oracle.end());
    std::vector<std::vector<double>>timings(algos.size());
    std::vector<size_t>order(algos.size());
    for(size_t i=0;i<order.size();++i)order[i]=i;
    for(int rep=0;rep<=reps;++rep){
     std::mt19937_64 shuffle_gen(seed^((U(rep)+1)*0x9e3779b97f4a7c15ULL));
     std::shuffle(order.begin(),order.end(),shuffle_gen);
     for(size_t id:order){
      auto a=input; // copy excluded from timing; sorter allocations included
      rune_v27::detail::compact_calls=0;
      auto start=Clock::now();
      algos[id].run(a);
      auto stop=Clock::now();
      if(a!=oracle){
       std::cerr<<"MISMATCH group="<<group<<" workload="<<cases[mi]<<" n="<<n
                <<" seed="<<seed<<" algo="<<algos[id].name<<"\n";
       return 5;
      }
      if(rep)timings[id].push_back(
        std::chrono::duration<double,std::milli>(stop-start).count());
     }
    }
    for(size_t id=0;id<algos.size();++id){
     const auto &raw=timings[id];
     auto ts=raw;
     std::sort(ts.begin(),ts.end());
     const double med=(ts[(ts.size()-1)/2]+ts[ts.size()/2])/2;
     std::cout<<group<<','<<cases[mi]<<','<<n<<','<<seed<<','<<algos[id].name<<','
        <<std::fixed<<std::setprecision(9)<<med<<','<<ts.front()<<','
        <<ts.back()<<','<<ts.size()<<",true,";
     for(double trial:raw)std::cout<<trial<<',';
     // Route is determined outside timing, without sorting a second time.
     // Reuse the sample/certificate on a separate copy for reporting.
     auto route_input=input;
     const bool selected=(id==0)&&rune_v27::detail::compact_count(route_input.data(),route_input.size());
     std::cout<<rune_v25::simd_available()<<','<<rune_v27::detail::bmi2_available()<<','<<selected<<"\n";
    }
    std::cout.flush();
   }
  }
 }catch(const std::exception&e){
  std::cerr<<"BENCH ERROR: "<<e.what()<<"\n";return 3;
 }
}
