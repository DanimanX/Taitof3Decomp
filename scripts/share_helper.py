#!/usr/bin/env python3
"""Point a file's private copy of a shared jsr callee at src/objects/obj_shared.h.

Each object routine used to carry its own `static` copy of the primitives that
now live in src/objects/obj_shared.c (see that header). Every copy is bit-exact
for its own file, but a private copy makes scripts/decomp_planner.py treat the
callee as "not decompiled" and blocks the whole dependent set.

Mapping (private name -> shared function):

  sub_0CBB2C -> Obj_ExplodeTick      sub_0CBE34 -> Obj_BlinkColour
  sub_0D0E30 -> Obj_RelayTimer       sub_0D0E3A -> Obj_Next4Way
  sub_0CF1D8 -> Obj_StartScript

For one file this script:
  1. finds `static ... sub_HEX(...) { ... }` and deletes the definition;
  2. rewrites every remaining call `sub_HEX(` in the file to the shared name;
  3. adds `#include "../objects/obj_shared.h"`.

Private math copies with their own signature (sub_006422/sub_00649A) are left
alone: some files use a void-with-out-parameters variant, and an in-file copy
is harmless as long as the shared one exists.

The translation never changes: only linkage and call-site names. Verify with
`F3_ONLY=ADDR ./build/test_objects` -- a green routine must stay green.

Usage:
  scripts/share_helper.py <ADDR> <private_fn> [--all] [--dry-run]
  scripts/share_helper.py 108936 sub_0CBB2C
  scripts/share_helper.py --all 0D0E3A
"""
import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(ROOT)

SHARED = {
    'sub_0CBB2C': 'Obj_ExplodeTick',
    'sub_0CBE34': 'Obj_BlinkColour',
    'sub_0D0E30': 'Obj_RelayTimer',
    'sub_0D0E3A': 'Obj_Next4Way',
    'sub_0CF1D8': 'Obj_StartScript',
    'sub_006422': 'Obj_Atan2',
    'sub_00649A': 'Obj_Distance',
}
MATH_FALLBACK = ()   # different files use different signatures; not automated
INCLUDE = '#include "../objects/obj_shared.h"'

DEF = r'^static [^\n]*?\b%s\([^\n]*\)\n\{'


def span(s, fn):
    """(start, end) of the definition, end exclusive; None if absent."""
    m = re.search(DEF % re.escape(fn), s, re.M)
    if not m:
        return None
    depth, k = 0, s.index('{', m.end() - 1)
    while k < len(s):
        if s[k] == '{':
            depth += 1
        elif s[k] == '}':
            depth -= 1
            if depth == 0:
                return m.start(), k + 1
        k += 1
    return None


# comma count of the shared signatures (obj-taking helpers have obj = A0 only)
ARITY = {'sub_0CBB2C': 1, 'sub_0CBE34': 1, 'sub_0D0E30': 1,
         'sub_0D0E3A': 0, 'sub_0CF1D8': 2}


def _shared_arity(fn):
    return ARITY.get(fn, -1)


def calls_to(s, fn):
    return len(re.findall(r'(?<![\w])' + re.escape(fn) + r'\(', s))


def normalise(arg):
    arg = arg if arg.startswith('sub_') else 'sub_' + arg
    for k in SHARED:
        if arg.lower() == k.lower():
            return k
    raise SystemExit(f'not a shared helper: {arg} (known: {", ".join(SHARED)})')


def files_for(addr, all_files):
    if all_files:
        return sorted(glob.glob('src/enemy/obj_*.c'))
    p = f'src/enemy/obj_{addr}.c'
    return [p] if os.path.exists(p) else []


def rewrite(path, fn, dry):
    new = SHARED[fn]
    s = open(path, encoding='utf-8').read()
    sp = span(s, fn)
    if sp is None:
        return False
    # a copy with its own signature (different arity/return type) is not the
    # shared primitive: leave it alone rather than silently changing meaning
    sig = re.search(DEF % re.escape(fn), s, re.M).group(0)
    if sig.count(',') != SHARED[fn].count(',') and '(' in sig:
        pass
    if re.search(r'return\s+' + re.escape(fn) + r'\(', s):
        return False
    if re.search(r'^static [^\n]*\b' + re.escape(new) + r'\(', s, re.M):
        return False                  # this file defines the shared name itself
    fnsig = re.search(DEF % re.escape(fn), s, re.M).group(0).split('\n')[0]
    if fnsig.count(',') != _shared_arity(fn):
        return False                  # different arity: not the same helper
    s2 = s[:sp[0]] + s[sp[1]:]
    removed = [fn]
    for old in removed:
        s2 = re.sub(r'(?<![\w])' + re.escape(old) + r'\(', new + '(', s2)
    s2 = re.sub(r'\n{3,}', '\n\n', s2)
    if INCLUDE not in s2:
        m = re.search(r'^#include [^\n]*\n', s2, re.M)
        s2 = s2[:m.end()] + INCLUDE + '\n' + s2[m.end():]
    if s2 != s and not dry:
        open(path, 'w', encoding='utf-8').write(s2)
    return s2 != s


def prune_math(path):
    """Remove private sub_0064xx copies left without a caller.

    The shared body calls Obj_Atan2/Obj_Distance directly, so a file whose only
    users of those private copies were the shared helpers no longer needs them;
    -Werror=unused-function would otherwise reject the build.
    """
    s = open(path, encoding='utf-8').read()
    before = s
    for m, shared in ('sub_006422', 'Obj_Atan2'), ('sub_00649A', 'Obj_Distance'):
        del shared
        sp = span(s, m)
        if sp is None:
            continue
        sp_now = span(s, m)
        head = s[sp_now[0]:sp_now[1]].split('\n')[0]
        if head.count(',') not in (1, 2):
            continue                    # different signature: another helper
        rest = s[:sp_now[0]] + s[sp_now[1]:]
        if 'Obj_ExplodeTick' not in rest:
            continue
        if calls_to(rest, m) == 0:
            s = rest
    s = re.sub(r'\n{3,}', '\n\n', s)
    if s != before:
        open(path, 'w', encoding='utf-8').write(s)
        return True
    return False


def main():
    argv = [a for a in sys.argv[1:] if not a.startswith('--')]
    dry = '--dry-run' in sys.argv
    all_files = '--all' in sys.argv
    if not argv:
        print(__doc__)
        return 1
    fn = normalise(argv[-1])
    addr = argv[0] if len(argv) > 1 else None
    files = files_for(addr, all_files)
    if not files:
        print(f'no file for {addr}', file=sys.stderr)
        return 1
    changed = [f for f in files if rewrite(f, fn, dry)]
    if not dry:
        for f in changed:
            prune_math(f)
    for f in changed[:5]:
        print(f'{fn} -> {SHARED[fn]}: {f}')
    if len(changed) > 5:
        print(f'... and {len(changed) - 5} more')
    print(f'{len(changed)}/{len(files)} file(s) rewritten' + (' [dry run]' if dry else ''))
    return 0


if __name__ == '__main__':
    sys.exit(main())
