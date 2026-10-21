#!/usr/bin/env python3
"""Fail if v24 changes any inherited decision except the single worker call."""
from pathlib import Path

root = Path(__file__).resolve().parents[2]
frozen = (root / 'runesort_v17/rune_sort.hpp').read_text()
expected = frozen[frozen.index('// Sorts uint64_t values'):
                  frozen.index('inline void sort(std::vector<uint64_t>& v)')]
expected = expected.replace('inline void sort(uint64_t* a, size_t n) {',
                            'template<unsigned Distance>\ninline void dispatch(uint64_t* a, size_t n) {')
before = '    detail::fixed_radix<8>(a,n);\n'
assert expected.count(before) == 1
expected = expected.replace(before, '#ifdef RUNE24_TRACE_PATH\n'
                            '    ++prefetch_route_calls;\n#endif\n'
                            '    rune_v23::radix16<Distance>(a,n);\n', 1)
actual = (root / 'runesort_v24/rune_sort.hpp').read_text().split(
    '// BEGIN INHERITED DISPATCHER\n', 1)[1].split('// END INHERITED DISPATCHER', 1)[0]
assert actual == expected, 'unreviewed inherited dispatcher change'
print('v24 inheritance verified: exactly one selected worker replacement')
