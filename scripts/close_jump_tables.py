#!/usr/bin/env python3
"""Replace the `if (k > 50) return -1;` guard in local copies of the angle
jump-table helper (ROM 0x0D4F26, used by 0x0D2D44 / 0x0D25E4) with the table's
actual meaning.

The table dispatches to 0x0D4FF2..0x0D538A, one block per k: block k scales
both 32-bit registers by k using shifts/adds (`* 2^15` constants), so
`(D0*k, D1*k) mod 2^32` is exact for k = 0..50. The guard was a placeholder
for that table; a copy that stops there reports a false "undecompiled path".

Each local copy is matched as a whole: the optional preceding one-line
comment, the guard (2 or 4 comment lines, `if (d2w > 50u) { return -1; }`)
and its 4/8-space indentation. Only the guard is replaced.
"""
import glob
import os
import re
import sys

PAT = re.compile(
    r"(?:(?P<pre>[ \t]*/\* Jump table 0x0D4F26 \[0\.\.50\] -> 0x0D4FF2\.\.0x0D538A:"
    r" exact \(D0\*k, D1\*k\) mod 2\^32\. \*/\n))?"
    r"(?P<ind>[ \t]*)if \(d2w > 50u\) \{\n"
    r"[ \t]*/\* unimplemented: ROM 0x0D4F26 \*/\n"
    r"[ \t]*return -1;\n"
    r"[ \t]*\}\n"
)

NEW = (
    "/* Jump table 0x0D4F26 [0..50] -> 0x0D4FF2..0x0D538A: block k returns the\n"
    "     * constant 0..50 (k=0 at 0x0D4FF2 clears both), so (D0*k, D1*k)\n"
    "     * mod 2^32 is exact for every k the table has. */\n"
)


def close(path):
    s = open(path).read()
    if NEW.strip().split("\n")[0] in s:
        return "closed"

    def repl(m):
        ind = m.group("ind")
        return "".join(ind + ln + "\n" for ln in NEW.rstrip("\n").split("\n"))

    out, n = PAT.subn(repl, s)
    if n == 0:
        return "no guard"
    open(path, "w").write(out)
    return f"closed {n}"


if __name__ == "__main__":
    if len(sys.argv) > 1:
        files = sys.argv[1:]
    else:
        root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        files = sorted(glob.glob(os.path.join(root, "src", "**", "*.c"), recursive=True))
    for f in files:
        r = close(f)
        if r != "no guard":
            print(f, r)
