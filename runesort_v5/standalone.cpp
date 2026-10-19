#include "rune_sort.hpp"
#include <cstdint>
#include <vector>
int main(){std::vector<std::uint64_t> a{9,2,7,2,5};rune::sort(a);return a==std::vector<std::uint64_t>{2,2,5,7,9}?0:1;}
