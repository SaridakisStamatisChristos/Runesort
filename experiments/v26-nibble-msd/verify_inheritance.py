#!/usr/bin/env python3
"""Check that every early exact shortcut remains the frozen v17 operation."""
from pathlib import Path
root = Path(__file__).resolve().parents[2]
source = (root/'runesort_v17/rune_sort.hpp').read_text()
start = source.index('// Sorts uint64_t values')
end = source.index('  if(n>=4096 && sampled_descents<=3', start)
expected = source[start:end].replace('inline void sort(uint64_t* a, size_t n) {',
    'template<bool Network>\ninline void selected_dispatch(uint64_t* a, size_t n) {')
before = '    detail::fixed_radix<8>(a,n);'
assert expected.count(before) == 1
expected = expected.replace(before, '#ifdef RUNE26_TRACE_PATH\n'
    '    ++selected_route_calls;\n#endif\n    kernel<Network>(a,n);')
expected = expected.replace('detail::', 'legacy::')
actual = (root/'runesort_v26/rune_sort.hpp').read_text().split(
    '// BEGIN INHERITED PREFIX\n')[1].split('// END INHERITED PREFIX')[0]
assert actual == expected, 'unexpected change to frozen early shortcuts'
print('v26 prefix inheritance verified')
