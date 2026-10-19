#include "rune_sort.hpp"
#include "rune_sort_v4.hpp"
#include <boost/sort/pdqsort/pdqsort.hpp>
#include <boost/sort/spreadsort/spreadsort.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>
using U=uint64_t;
using clk=std::chrono::steady_clock;
std::vector<U> generate(const std::string& cas,size_t n,uint64_t seed){
 std::mt19937_64 rng(seed);std::vector<U>a(n);
 if(cas=="local"){
 for(auto &x:a)x=rng();std::sort(a.begin(),a.end());
 for(size_t i=0;i<n/100;++i){size_t j=rng()%(n-1);std::swap(a[j],a[j+1]);}
 }else if(cas=="nearly"){
 for(auto &x:a)x=rng();std::sort(a.begin(),a.end());
 for(size_t i=0;i<n/100;++i)std::swap(a[rng()%n],a[rng()%n]);
 }else if(cas=="rotated"){
 for(auto&x:a)x=rng();std::sort(a.begin(),a.end());std::rotate(a.begin(),a.begin()+n/3,a.end());
 }else if(cas=="uniform"){
 for(auto&x:a)x=rng();
 }else if(cas=="random32"){
 for(auto&x:a)x=rng()&0xffffffffULL;
 }else if(cas=="random48"){
 for(auto&x:a)x=rng()&0xffffffffffffULL;
 }else if(cas=="random56"){
 for(auto&x:a)x=rng()&0xffffffffffffffULL;
 }else if(cas=="sparse16"){
 U mask=(1ULL<<0)|(1ULL<<4)|(1ULL<<9)|(1ULL<<13)|(1ULL<<18)|(1ULL<<23)|(1ULL<<27)|(1ULL<<32)|(1ULL<<36)|(1ULL<<41)|(1ULL<<45)|(1ULL<<49)|(1ULL<<53)|(1ULL<<57)|(1ULL<<61)|(1ULL<<63);
 for(auto&x:a)x=rune::detail::deposit(rng()&65535ULL,mask)^0x22222222ULL;
 }else if(cas=="zipf"){
 for(auto&x:a)x=1ULL<<(rng()%64);
 }else if(cas=="narrow"){
 for(auto&x:a)x=0x8000000000000000ULL+(rng()&65535ULL);
 }else if(cas=="sorted"){
 for(auto&x:a)x=rng();std::sort(a.begin(),a.end());
 }else if(cas=="reverse"){
 for(auto&x:a)x=rng();std::sort(a.begin(),a.end(),std::greater<U>());
 }else if(cas=="equal"){
 for(auto&x:a)x=0xdeadbeef;
 }else if(cas=="alternating"){
 for(size_t i=0;i<n;++i)a[i]=(i&1)?~U(0):0;
 }else {
 size_t cardinality=std::stoul(cas.substr(1));std::vector<U>choices(cardinality);
 for(auto&x:choices)x=rng();for(auto&x:a)x=choices[rng()%cardinality];
 }
 return a;
}
int main(int argc,char**argv){size_t n=argc>1?std::stoull(argv[1]):1000000;int reps=argc>2?std::stoi(argv[2]):5;uint64_t seed=argc>3?std::stoull(argv[3]):12345;
 std::vector<std::string> modes={"uniform","random32","random48","random56","sparse16","zipf","narrow","sorted","reverse","equal","alternating","local","nearly","rotated","K16","K64","K128","K512","K1024","K2048","K4096","K8192"};
 if(argc>4){modes.clear();std::stringstream ss(argv[4]);std::string item;while(std::getline(ss,item,','))if(!item.empty())modes.push_back(item);}
 std::cout<<"mode,n,seed,v5_ms,v4_ms,pdqsort_ms,spreadsort_ms,std_sort_ms\n";
 for(size_t mi=0;mi<modes.size();++mi){auto mode=modes[mi];auto input=generate(mode,n,seed+mi);auto expected=input;std::sort(expected.begin(),expected.end());
 std::array<std::vector<double>,5> times{};
 for(int r=0;r<reps+1;++r)for(int shift=0;shift<5;++shift){int algo=(shift+r)%5;auto v=input;
 auto t0=clk::now();
 if(algo==0)rune::sort(v);
 if(algo==1)rune_v4::sort(v);
 if(algo==2)boost::sort::pdqsort(v.begin(),v.end());
 if(algo==3)boost::sort::spreadsort::spreadsort(v.begin(),v.end());
 if(algo==4)std::sort(v.begin(),v.end());
 auto t1=clk::now();
 if(v!=expected){std::cerr<<"FAIL "<<mode<<" algorithm "<<algo<<"\n";return 5;}
 if(r)times[algo].push_back(std::chrono::duration<double,std::milli>(t1-t0).count());}
 std::cout<<mode<<","<<n<<","<<seed;
 for(auto &t:times){std::sort(t.begin(),t.end());std::cout<<","<<std::setprecision(5)<<std::fixed<<t[t.size()/2];}std::cout<<"\n";
 }
}
