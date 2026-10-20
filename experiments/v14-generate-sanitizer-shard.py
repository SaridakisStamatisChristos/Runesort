#!/usr/bin/env python3
"""Shard the canonical v14 adversarial test without silently changing its assertions.

The generated translation unit is identical to test_spectrum.cpp except that
a single selected mask pattern (0..5) executes in each job. Six sharded jobs
therefore cover all original width/size/mask combinations. The full unsharded
test is still exercised in native Release CTest, unchanged.
"""
from pathlib import Path
import sys

SOURCE = Path("runesort_v14/test_spectrum.cpp")
MAIN = "int main(){"
LOOP = "for(int p=0;p<6;++p){"

def main() -> int:
    if len(sys.argv) != 3:
        raise SystemExit("usage: v14-generate-sanitizer-shard.py OUTPUT.cpp PATTERN_ID")
    output = Path(sys.argv[1])
    pattern = int(sys.argv[2])
    if pattern not in range(6):
        raise SystemExit(f"pattern must be 0..5: {pattern}")
    source = SOURCE.read_text(encoding="utf-8")
    if source.count(MAIN) != 1 or source.count(LOOP) != 1:
        raise SystemExit("Canonical source layout changed; manual review required.")
    source = source.replace(
        MAIN, f"int main(){{\n constexpr int kPattern = {pattern};", 1
    ).replace(
        LOOP, LOOP + "\n   if(p!=kPattern)continue;", 1
    )
    output.write_text(source, encoding="utf-8")
    print(f"Generated sharded canonical spectrum suite: pattern={pattern}; target={output}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
