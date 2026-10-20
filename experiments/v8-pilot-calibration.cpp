// Reproduce the exact duplicate-count sample experiment behind the v8 pilot.
// This is a standalone calibration diagnostic, NOT a sorting-speed benchmark.
// Copyright (c) 2026 Stamatis-Christos Saridakis. All rights reserved.
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>
using U=std::uint64_t;
static size_t duplicates(const std::vector<U>& array, size_t count){
  std::unordered_set<U> keys;
  keys.reserve(count*2);
  for(size_t i=0;i<count;++i)keys.insert(array[(i*array.size())/count]);
  return count-keys.size();
}
int main(int argc,char**argv){
  if(argc!=2||(std::string(argv[1])!="calibration"&&std::string(argv[1])!="heldout")){
    std::cerr<<"Usage: pilot_calibration calibration|heldout\n";return 2;
  }
  const bool heldout=std::string(argv[1])=="heldout";
  constexpr size_t N=1000000;
  const size_t K[]={65536,67584,69632,71680,72704,73728,74752,76800,
                    77824,79872,81920,83968,86016,90112,98304};
  const U seeds_calibration[]={271828,314159,777777,8675309,
      95101,95102,95103,95104,95105,95106,95107,95108,
      95109,95110,95111,95112,95113,95114,95115,95116};
  const U seeds_heldout[]={110001,110002,110003,110004,110005,
      110006,110007,110008,110009,110010,110011,110012,
      110013,110014,110015,110016,110017,110018,110019,110020};
  std::vector<U> a(N);
  std::cout<<"k,seed,d16,d32\n";
  for(size_t k:K){
    for(size_t j=0;j<20;++j){
      U seed=heldout?seeds_heldout[j]:seeds_calibration[j];
      std::mt19937_64 rng(seed+987);
      std::vector<U> keys(k);
      for(auto&v:keys)v=rng();
      for(auto&x:a)x=keys[rng()%k];
      std::cout<<k<<","<<seed<<","
               <<duplicates(a,16384)<<","<<duplicates(a,32768)<<"\n";
    }
  }
  return 0;
}
