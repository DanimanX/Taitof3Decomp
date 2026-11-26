#!/usr/bin/env python3
"""Draft register-level C for captured routines with the local Strata model,
verify each against its MAME capture with test_fn (FN1 mode), retry once with
the compiler/mismatch output, and accept bit-exact ones into src/auto/.

Usage: scripts/strata_fn.py [--max-size N] [--limit N] [--retries N] [ADDR ...]
  without ADDRs: every row of work/zcap/todo.tsv that has a capture and a
  bundle, smallest-callee-depth first (leaves first), up to --max-size bytes.
Results: work/zcap/strata_results.tsv (addr, size, verdict, detail).
Accepted: src/auto/fn_ADDR.c, prototype in src/auto/fn_auto.h, entry in
tests/fn_auto.inc. Nothing is committed.
A capture that a do-nothing draft also passes is marked 'weak' and never
accepted: it cannot tell a translation from an empty function."""
import json, os, re, subprocess, sys, urllib.request, glob, argparse, time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(ROOT)
URL = os.environ.get("STRATA_URL", "http://127.0.0.1:8080") + "/v1/chat/completions"
DR = "work/zcap/drafts"
THINK = os.environ.get("STRATA_THINK", "1") == "1"
os.makedirs(DR, exist_ok=True)
CFLAGS = ["-std=c11", "-O1", "-Wall", "-Wextra", "-Werror", "-Wno-unused-function",
          "-iquote", "src/auto"]

def b(a): return a.lstrip("0") or "0"

def bundle(a):
    p = f"work/disasm/bundle_{b(a)}.s"
    return open(p).read() if os.path.exists(p) else None

HDRS = None
def helper_protos(text):
    """Prototype lines (and their header) for every STOP callee in a bundle."""
    global HDRS
    if HDRS is None:
        HDRS = {}
        for h in glob.glob("src/**/*.h", recursive=True):
            for ln in open(h, errors="ignore"):
                for x in re.findall(r"ROM 0x([0-9A-Fa-f]{4,6})", ln):
                    if "(" in ln and ";" in ln:
                        HDRS.setdefault(int(x, 16), (h, ln.strip()))
    out = []
    m = re.search(r"^; calls:(.*)$", text, re.M)
    for x in re.findall(r"([0-9A-F]{6}) STOP", m.group(1) if m else ""):
        h = HDRS.get(int(x, 16))
        out.append(f"0x{x}: " + (f'#include "../{h[0][4:]}"  ->  {h[1]}' if h else "decompiled but no header prototype found; return -1 on paths that call it"))
    return "\n".join(out)

def regs_sample(a, n=2):
    p = f"work/ram/fn_{a}_regs.tsv"
    if not os.path.exists(p): return ""
    ls = open(p).read().splitlines()
    return "\n".join(ls[: 1 + 2 * n])

def npairs(a):
    p = f"work/ram/fn_{a}_regs.tsv"
    return sum(1 for l in open(p) if "\tpost\t" in l) if os.path.exists(p) else 0

REF = "".join(open("src/common/fn_leaf1.c").readlines()[:75])
PIT = open("work/zcap/pitfalls.txt").read()

SYSTEM = "You translate Motorola 68020 assembly to exact, compilable C11. Output one ```c code block and nothing else."

def prompt(a, text):
    return f"""Translate ROM routine 0x{a} (Darius Gaiden, Taito F3, MC68EC020) to C, register-faithful.

Write ONE C file with exactly this entry point:
    int Fn_{a}(const f3mem_t *m, f3regs_t *r)
- r->d[0..7], r->a[0..7] are the 68k registers at entry (a[7] = SP; stack args at SP+4..). On return, leave every register exactly as the ROM leaves it at its rts (a[7] back to the entry SP). Return 0. Return -1 (before any side effect of that path) if a path needs a routine you cannot translate.
- Memory: rd8/rd16/rd32/wr8/wr16/wr32(m, addr) from "../common/f3mem.h" (big-endian, any address: Work RAM 0x400000, ROM < 0x200000 reads the real ROM). F3_A5 = 0x408000; d16(A5) offsets are SIGNED.
- Start the file with: #include "fn_auto.h"   (it includes f3mem.h and defines f3regs_t).
- Callees inside the listing that are not marked STOP: translate them as static functions in this same file (register-level, same style). For jsr/bsr: r->a[7] -= 4; wr32(m, r->a[7], <return address>); call; r->a[7] += 4.
- Decompiled callees (STOP) and their C helpers:
{helper_protos(text) or "(none)"}
- .b/.w ops change only the low byte/word of a data register: r->d[0] = (r->d[0] & 0xFFFF0000u) | w. Keep condition codes in mind for Bcc after arithmetic; signed vs unsigned compares.
- No main(), no printf. Comment each block with its ROM address.

Known pitfalls:
{PIT}

Style reference (verified file in this project):
```c
{REF}
```

Captured registers (first calls; columns pair which frame D0-D7 A0-A7):
{regs_sample(a)}

Disassembly (the routine first, then its callees):
```
{text}
```"""

def ask(msgs, think=None):
    think = THINK if think is None else think
    body = {"model": "qwen3.6-35b-a3b", "messages": msgs, "temperature": 0.2, "max_tokens": 32000,
            "stream": False, "chat_template_kwargs": {"enable_thinking": think}, "reasoning_effort": "medium" if think else "none"}
    req = urllib.request.Request(URL, json.dumps(body).encode(), {"Content-Type": "application/json"})
    with urllib.request.urlopen(req, timeout=900) as f:
        out = json.load(f)["choices"][0]["message"]["content"] or ""
    return re.sub(r"<think>.*?</think>\s*", "", out, flags=re.S)

def code_of(reply):
    m = re.findall(r"```(?:c|C)?\n(.*?)```", reply, re.S)
    return max(m, key=len) if m else None

def build_run(a, src):
    exe = f"build/fn1_{a}"
    # the file must also build under the project's own flags (no -Wno-unused-function)
    c0 = subprocess.run(["cc", "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror", "-iquote", "src/auto",
                         "-c", src, "-o", "/dev/null"], capture_output=True, text=True)
    if c0.returncode:
        return None, "COMPILE ERROR (project flags):\n" + (c0.stderr or c0.stdout)[:3000]
    c = subprocess.run(["cc", *CFLAGS, f"-DFN1=Fn_{a}", f'-DFN1_ADDR="{a}"', "-o", exe,
                        "tests/test_fn.c", src, "build/libf3.a"], capture_output=True, text=True)
    if c.returncode:
        return None, "COMPILE ERROR:\n" + (c.stderr or c.stdout)[:3000]
    try:
        r = subprocess.run([exe], capture_output=True, text=True, timeout=120)
        out = r.stdout + r.stderr[:1500]
    except subprocess.TimeoutExpired:
        return None, "TIMEOUT: the translation loops forever on the captured input"
    m = re.search(r"(\d+)/\s*(\d+) calls bit-exact \((\d+) undecompiled", out)
    if not m: return None, out[-2500:]
    return (int(m.group(1)), int(m.group(2)), int(m.group(3))), out[-2500:]

def null_passes(a):
    p = f"{DR}/null_{a}.c"
    open(p, "w").write(f'#include "fn_auto.h"\nint Fn_{a}(const f3mem_t *m, f3regs_t *r) {{ (void)m; (void)r; return 0; }}\n')
    res, _ = build_run(a, p)
    return bool(res and res[1] > 0 and res[0] == res[1])

def accept(a, src):
    code = open(src).read()
    dst = f"src/auto/fn_{a}.c"
    open(dst, "w").write(f"/* ROM 0x{a}: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */\n" + code)
    h = open("src/auto/fn_auto.h").read()
    proto = f"int Fn_{a}(const f3mem_t *m, f3regs_t *r); /* ROM 0x{a} */\n"
    if proto not in h: h = h.replace("/* END generated */", proto + "/* END generated */")
    open("src/auto/fn_auto.h", "w").write(h)
    inc = open("tests/fn_auto.inc").read()
    line = f'    {{ "Fn 0x{a} auto", "work/ram/fn_{a}", Fn_{a}, ALLREGS }},\n'
    if line not in inc: open("tests/fn_auto.inc", "a").write(line)
    subprocess.run(["make", "-s", "-j16", "build/libf3.a"], capture_output=True)

def depth(text):
    m = re.search(r"^; calls:(.*)$", text, re.M)
    calls = m.group(1) if m else ""
    return len(re.findall(r"[0-9A-F]{6} (?!STOP)", calls))

def log(row):
    with open("work/zcap/strata_results.tsv", "a") as f: f.write("\t".join(map(str, row)) + "\n")
    print("\t".join(map(str, row)), flush=True)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("addrs", nargs="*")
    ap.add_argument("--max-size", type=int, default=300)
    ap.add_argument("--limit", type=int, default=30)
    ap.add_argument("--retries", type=int, default=1)
    ap.add_argument("--min-pairs", type=int, default=3)
    o = ap.parse_args()
    done = set()
    if os.path.exists("work/zcap/strata_results.tsv"):
        done = {l.split("\t")[0] for l in open("work/zcap/strata_results.tsv")}
    sizes = {l.split()[0]: int(l.split()[1]) for l in open("work/zcap/todo.tsv")}
    if o.addrs:
        todo = [x.upper().zfill(6) for x in o.addrs]
    else:
        cand = [a for a, s in sizes.items() if s <= o.max_size and a not in done
                and npairs(a) >= o.min_pairs and bundle(a) and not os.path.exists(f"src/auto/fn_{a}.c")]
        todo = sorted(cand, key=lambda a: (depth(bundle(a)), sizes[a]))[: o.limit]
    subprocess.run(["make", "-s", "-j16", "build/libf3.a"], check=True)
    for a in todo:
        t0 = time.time()
        text = bundle(a)
        if not text: log([a, sizes.get(a, 0), "nobundle", ""]); continue
        if null_passes(a): log([a, sizes.get(a, 0), "weak", "empty draft passes the capture"]); continue
        msgs = [{"role": "system", "content": SYSTEM}, {"role": "user", "content": prompt(a, text)}]
        verdict, detail = "fail", ""
        nothink = False
        for attempt in range(2 + o.retries):
            if attempt == 1 + o.retries and not nothink: break
            try:
                reply = ask(msgs, think=False if nothink else None)
            except Exception as e:
                verdict, detail = "error", repr(e)[:200]; break
            code = code_of(reply)
            src = f"{DR}/fn_{a}.c"
            if not code:
                # thinking ran out of tokens before any code: ask again without thinking
                detail = "no code block"; nothink = True; continue
            open(src, "w").write(code)
            res, out = build_run(a, src)
            if res and res[1] >= o.min_pairs and res[0] == res[1] and res[2] == 0:
                accept(a, src); verdict = "accepted"; detail = f"{res[0]}/{res[1]} undec={res[2]} try={attempt}"; break
            detail = (f"{res[0]}/{res[1]} undec={res[2]}" if res else (out.strip().splitlines() or ["no output"])[0][:80]) + f" try={attempt}"
            msgs += [{"role": "assistant", "content": "```c\n" + code + "```"},
                     {"role": "user", "content": "test_fn against the MAME capture says:\n" + out +
                      "\nFix the translation (re-read the bytes for each mismatching address/register) and reply with the whole corrected ```c file."}]
        log([a, sizes.get(a, 0), verdict, detail + f" {int(time.time() - t0)}s"])

if __name__ == "__main__":
    main()
