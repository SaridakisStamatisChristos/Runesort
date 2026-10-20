#!/usr/bin/env python3
"""Check x86 v14-v13 semantic/codegen identity for identical entrypoint builds.

Both sources are included separately into an otherwise identical translation
unit, compiled sequentially *from the same path* with the same compiler/flags.
Compare canonical preprocessed C++ and release object bytes. Never conflate this
with a benchmark or a correctness proof for ARM64.
"""
from __future__ import annotations
import difflib
import hashlib
from pathlib import Path
import shutil
import subprocess
import tempfile

HEADERS = ["runesort_v13/rune_sort.hpp", "runesort_v14/rune_sort.hpp"]
CONFIGS = [
    ("native", ["-O3", "-march=native"]),
    ("portable_no_bmi2", ["-O3", "-march=x86-64", "-mno-bmi2"]),
]

def run(args: list[str]) -> bytes:
    return subprocess.check_output(args, stderr=subprocess.PIPE)

def check(compiler: str, profile: str, extra: list[str]) -> None:
    with tempfile.TemporaryDirectory(prefix="rune_x86_identity_") as td:
        path = Path(td) / "identical_source.cpp"
        obj = Path(td) / "same_output.o"
        tokens = []
        objects = []
        for header in HEADERS:
            path.write_text(
                f'#include "{header}"\n'
                'extern "C" void runesort_entry(unsigned long long* a, unsigned long long n) {\n'
                '    rune::sort(reinterpret_cast<uint64_t*>(a), static_cast<size_t>(n));\n'
                '}\n',
                encoding="utf-8",
            )
            opts = [compiler, "-std=c++17", *extra, "-DNDEBUG",
                    "-fno-ident", "-g0", "-I."]
            preprocessed = run([*opts, "-E", "-P", str(path)]).decode("utf-8")
            # Only strip blank lines: token boundaries remain intact.
            tokens.append([line.strip() for line in preprocessed.splitlines() if line.strip()])
            run([*opts, "-c", str(path), "-o", str(obj)])
            objects.append(obj.read_bytes())
        if tokens[0] != tokens[1]:
            diff = "\n".join(difflib.unified_diff(
                tokens[0], tokens[1], fromfile=HEADERS[0], tofile=HEADERS[1], n=2
            ))
            raise SystemExit(f"PREPROCESSOR DIFFERENCE: {compiler} {profile}\n{diff[:8000]}")
        if objects[0] != objects[1]:
            raise SystemExit(f"OBJECT DIFFERENCE: {compiler} {profile}, despite identical preprocessed C++")
        sha = hashlib.sha256(objects[0]).hexdigest()
        print(f"PASS compiler={compiler} profile={profile} preprocessed_identical=true "
              f"object_identical=true object_sha256={sha}")

def main() -> None:
    compilers = ["g++"]
    if shutil.which("clang++"):
        compilers.append("clang++")
    for compiler in compilers:
        for profile, extra in CONFIGS:
            check(compiler, profile, extra)

if __name__ == "__main__":
    main()
