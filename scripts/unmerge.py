#!/usr/bin/env python3
"""unmerge.py DEST ADDR... : take merged src/auto routines back out of main
(move src/auto/fn_ADDR.c to DEST, drop its prototype from src/auto/fn_auto.h
and its line from tests/fn_auto.inc). Used by agy_loop.sh to hold back a
routine that a check flagged after agy_merge.sh merged it."""
import os, shutil, sys

os.chdir(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
dest, addrs = sys.argv[1], sys.argv[2:]
os.makedirs(dest, exist_ok=True)
h = open("src/auto/fn_auto.h").read().splitlines(True)
inc = open("tests/fn_auto.inc").read().splitlines(True)
for a in addrs:
    src = f"src/auto/fn_{a}.c"
    if os.path.exists(src):
        shutil.move(src, os.path.join(dest, f"fn_{a}.c"))
    h = [l for l in h if f"Fn_{a}(" not in l]
    inc = [l for l in inc if f'fn_{a}"' not in l]
open("src/auto/fn_auto.h", "w").writelines(h)
open("tests/fn_auto.inc", "w").writelines(inc)
print("held back:", " ".join(addrs))
