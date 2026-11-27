#!/usr/bin/env python3
"""Which ROM routines' C ran during the bit-exact test suites.

Build: scripts/gcov_build.sh (instrumented test_fn/test_objects, run, gcov JSON
in build/gcovj). For every C function with execution_count > 0, the ROM
addresses are taken from its name (6 hex digits) and from the "ROM 0x..."
comment just above its definition. Writes work/gcov_exec.tsv (addr, C name,
file, count). coverage.py counts these as verified: their C was executed
inside differential tests that pass bit-exact against MAME captures.
"""
import glob, gzip, json, os, re
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
hit = {}
lines_cache = {}
def src_lines(p):
    if p not in lines_cache:
        try: lines_cache[p] = open(p).read().split("\n")
        except OSError: lines_cache[p] = []
    return lines_cache[p]
for j in glob.glob(f"{ROOT}/build/gcovj/*.gcov.json.gz"):
    d = json.load(gzip.open(j))
    for f in d["files"]:
        path = os.path.normpath(os.path.join(d.get("current_working_directory", ""), f["file"]))
        if "/tests/" in path: continue
        L = src_lines(path)
        for fn in f["functions"]:
            if fn["execution_count"] <= 0: continue
            addrs = set(re.findall(r"([0-9A-F]{6})", fn["name"]))
            s = fn["start_line"] - 1
            # comment block above the definition (stop at the previous '}' or ';')
            k = s - 1
            while k >= 0 and k > s - 12:
                t = L[k].strip()
                if t.startswith("}") or (t.endswith(";") and not t.startswith("*")): break
                for lst in re.findall(r"ROM (0x[0-9A-Fa-f]{4,6}(?:,\s*0x[0-9A-Fa-f]{4,6})*)", L[k]):
                    addrs |= {x.upper().zfill(6) for x in re.findall(r"0x([0-9A-Fa-f]{4,6})", lst)}
                k -= 1
            # same-line or first-lines comment inside the body
            for t in L[s:s + 2]:
                for lst in re.findall(r"ROM (0x[0-9A-Fa-f]{4,6}(?:,\s*0x[0-9A-Fa-f]{4,6})*)", t):
                    addrs |= {x.upper().zfill(6) for x in re.findall(r"0x([0-9A-Fa-f]{4,6})", lst)}
            for a in addrs:
                a = a.zfill(6).upper()
                if a not in hit or hit[a][2] < fn["execution_count"]:
                    hit[a] = (fn["name"], os.path.relpath(path, ROOT), fn["execution_count"])
with open(f"{ROOT}/work/gcov_exec.tsv", "w") as o:
    o.write("# ROM routine whose C ran in the passing bit-exact suites (scripts/gcov_exec.py)\n")
    for a in sorted(hit):
        o.write(f"{a}\t{hit[a][0]}\t{hit[a][1]}\t{hit[a][2]}\n")
print(len(hit), "routines executed")
