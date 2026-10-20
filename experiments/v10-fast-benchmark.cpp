#include "runesort_v10/rune_sort.hpp"
#define rune rune_v7
#include "runesort_v7/rune_sort.hpp"
#undef rune
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
using U=uint64_t;
using clock_type=std::chrono::steady_clock;
int main(){
 const std::array<uint64_t,5> seeds={20261101,20261103,20261107,20261109,20261113};
 const std::array<int,11> ks={73728,81920,90112,98304,114688,
                               131072,147456,200000,0,-1,-2};
 const size_t n=1000000;
 std::cout<<"keys,n,seed,algorithm,median_ms,correct\n";
 for(int k:ks)for(U seed:seeds){
   std::mt19937_64 rng(seed);std::vector<U> original(n);
   if(k>0){std::vector<U> keys(k);for(auto&x:keys)x=rng();for(auto&x:original)x=keys[rng()%k];}
   else for(auto&x:original)x=k==0?rng():k==-1?rng()&0xffffffffULL:rng()&65535ULL;
   auto oracle=original;std::sort(oracle.begin(),oracle.end());
   std::array<std::vector<double>,2> timings{};
   for(int rep=0;rep<5;++rep){
     std::array<int,2> order={0,1};
     std::mt19937_64 gen(seed+991*rep);
     std::shuffle(order.begin(),order.end(),gen);
     for(int alg:order){
       auto a=original;auto t0=clock_type::now();
       if(alg==0)rune::sort(a);else rune_v7::sort(a);
       auto t1=clock_type::now();
       if(a!=oracle){std::cerr<<"WRONG SORT k="<<k<<" seed="<<seed<<" alg="<<alg<<"\n";return 2;}
       if(rep>0)timings[alg].push_back(std::chrono::duration<double,std::milli>(t1-t0).count());
     }
   }
   for(int alg=0;alg<2;++alg){
     auto& v=timings[alg];std::sort(v.begin(),v.end());
     std::cout<<k<<','<<n<<','<<seed<<','<<(alg==0?"RUNE_v10":"RUNE_v7")<<','
       <<(v[1]+v[2])/2.0<<",true\n";
   }
   std::cout.flush();
 }
}
