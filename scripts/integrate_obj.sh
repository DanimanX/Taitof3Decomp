#!/usr/bin/env bash
# Integrate a drafted object tick routine: header, Makefile SRC, test entry.
# Usage: scripts/integrate_obj.sh <ADDR> <draft.c>
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
A=$1
cp "$2" "$ROOT/src/enemy/obj_$A.c"
printf '#ifndef TAITOF3_OBJ_%s_H\n#define TAITOF3_OBJ_%s_H\n#include "../common/f3mem.h"\n/* Object tick routine (A0 = obj). Returns 0, or -1 when it reaches a ROM\n * path not decompiled yet (no side effects of that path are applied). */\nint Obj_%s_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x%06X */\n#endif\n' "$A" "$A" "$A" "$((16#$A))" > "$ROOT/src/enemy/obj_$A.h"
python3 - "$ROOT" "$A" <<'PY'
import sys
root, a = sys.argv[1], sys.argv[2]
p = root + '/tests/test_objects.c'; s = open(p).read()
inc = '#include "../src/enemy/obj_%s.h"' % a
if inc not in s:
    s = s.replace('#include "../src/objects/objscript_CF1E4.h"', inc + '\n#include "../src/objects/objscript_CF1E4.h"')
    marker = '    /* captured with F3_EXIT=ret'
    s = s.replace(marker, '    bad += run("Obj 0x%06X", "work/ram/obj_%s", Obj_%s_Tick, rom);\n' % (int(a, 16), a, a) + marker)
    open(p, 'w').write(s)
p = root + '/Makefile'; s = open(p).read()
src = 'src/enemy/obj_%s.c' % a
if src not in s:
    s = s.replace('src/enemy/obj_EAD28.c', 'src/enemy/obj_EAD28.c ' + src, 1)
    open(p, 'w').write(s)
PY
cc -std=c11 -O2 -Wall -Wextra -Werror -c "$ROOT/src/enemy/obj_$A.c" -o /dev/null
