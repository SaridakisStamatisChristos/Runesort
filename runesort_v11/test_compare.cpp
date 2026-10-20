#include "rune_sort.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

using U = std::uint64_t;
static void verify(std::vector<U> values) {
    auto reference=values;
    std::sort(reference.begin(),reference.end());
    rune::sort(values);
    if (values!=reference) { std::cerr<<"sorting mismatch at size "<<values.size()<<"\n"; std::exit(1); }
}
int main() {
    std::mt19937_64 g(8119);
    for (size_t n: {size_t(0),size_t(1),size_t(2),size_t(7),size_t(63),size_t(64),size_t(65),size_t(255),size_t(256),size_t(511),size_t(1024),size_t(4096),size_t(16384)}) {
        for(int trial=0;trial<50;++trial) {
            std::vector<U> a(n);
            switch(trial%12) {
            case 0: for(auto& x:a)x=g();break;
            case 1: for(auto& x:a)x=g()&0xffffffffULL;break;
            case 2: for(auto& x:a)x=g()&65535ULL;break;
            case 3: for(auto& x:a)x=g()%16;break;
            case 4: for(auto& x:a)x=(g()%16384)*0x9e3779b97f4a7c15ULL;break;
            case 5: for(auto& x:a)x=~U(0);break;
            case 6: for(size_t i=0;i<n;++i)a[i]=i;break;
            case 7: for(size_t i=0;i<n;++i)a[i]=n-i;break;
            case 8: for(size_t i=0;i<n;++i)a[i]=(i&1)?~U(0):0;break;
            case 9: for(auto& x:a)x=g();std::sort(a.begin(),a.end());break;
            case 10: for(auto& x:a)x=g();std::sort(a.begin(),a.end());if(n)std::rotate(a.begin(),a.begin()+n/3,a.end());break;
            default: for(auto& x:a)x=g();if(n>64) { for(size_t i=0;i<64;++i)a[i]=i; a.back()=~U(0); }break;
            }
            verify(a);
        }
    }
    for(int trial=0;trial<2500;++trial) {
        size_t n=64+(g()%8192);
        std::vector<U>a(n);
        U mask=0;
        unsigned bits=1+(g()%64);
        while(rune::detail::popcount(mask)<bits) mask|=U(1)<<(g()%64);
        const U base=g()&~mask;
        for(auto&x:a)x=base|rune::detail::deposit(g(),mask);
        if(trial%3==0)std::sort(a.begin(),a.end());
        if(trial%7==0 && n>0)std::rotate(a.begin(),a.begin()+n/3,a.end());
        if(trial%13==0)for(size_t i=0;i<n/100;++i)std::swap(a[g()%n],a[g()%n]);
        verify(a);
    }
    std::cout<<"PASSED 3150 differential test cases\n";
}
