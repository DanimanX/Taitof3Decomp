#!/usr/bin/env python3
"""Draft register-level C for a second Taito F3 game (default elvactr) with the
local Strata model, check each draft bit-exact against its MAME capture
(<game>/work/ram/fn_<ADDR>_*) and accept it into <game>/src/ea_<ADDR>.c.

Differences from scripts/strata_fn.py (dariusg):
- the listing comes from capstone over the Ghidra function size
  (<game>/work/functions.tsv), plus one level of callees not translated yet,
  which the draft must translate as static helpers in the same file;
- callees already accepted are called through their Ea_<ADDR> prototype;
- when scripts/match_fuzzy.py found a dariusg relative (score >= 0.7) whose
  verified C exists in src/, that C and its disassembly are given as the
  base to port (same code at other addresses).
A capture that an empty function also passes is 'weak' and never accepted.
Usage: scripts/strata_game.py [--game elvactr] [--limit N] [--max-size N] [ADDR ...]
Results: <game>/work/strata_results.tsv. Nothing is committed."""
import argparse, glob, json, os, re, subprocess, time, urllib.request, capstone

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(ROOT)
URL = os.environ.get("STRATA_URL", "http://127.0.0.1:8080") + "/v1/chat/completions"
THINK = os.environ.get("STRATA_THINK", "1") == "1"
md = capstone.Cs(capstone.CS_ARCH_M68K, capstone.CS_MODE_BIG_ENDIAN | capstone.CS_MODE_M68K_020)
md.detail = True
DROM = open("work/maincpu.bin", "rb").read()
SYSTEM = "You translate Motorola 68020 assembly to exact, compilable C11. Output one ```c code block and nothing else."
REF = open("src/auto/fn_004216.c").read().replace("Fn_004216", "Ea_004216").replace('"fn_auto.h"', '"ea_auto.h"')
PIT = open("work/zcap/pitfalls.txt").read() if os.path.exists("work/zcap/pitfalls.txt") else ""


def load_funcs(p):
    fs = {}
    for ln in open(p):
        q = ln.rstrip("\n").split("\t")
        if not q[0].startswith("#"):
            fs[int(q[0], 16)] = int(q[1])
    return fs


def disasm(rom, a, n, title):
    lines = [f"{title}:"]
    for i in md.disasm(rom[a:a + n + 16], a):
        if i.address >= a + n:
            break
        note = ""
        try:
            for op in i.operands:
                if op.type == capstone.m68k.M68K_OP_MEM and op.mem.index_reg and op.mem.scale > 1:
                    note = f"   ; index *{op.mem.scale}"
        except Exception:
            pass
        lines.append(f"  {i.address:06X}  {i.mnemonic:<8} {i.op_str}{note}")
    return "\n".join(lines)


def callees(rom, a, n):
    out, indirect = set(), False
    for i in md.disasm(rom[a:a + n + 16], a):
        if i.address >= a + n:
            break
        if i.mnemonic.startswith(("jsr", "bsr")):
            m = re.fullmatch(r"\$([0-9a-f]+)(\.l|\.w)?", i.op_str.strip())
            if m:
                out.add(int(m.group(1), 16))
            else:
                indirect = True
    return out, indirect


class Game:
    def __init__(self, g):
        self.g = g
        self.rom = open(f"{g}/work/maincpu.bin", "rb").read()
        self.funcs = load_funcs(f"{g}/work/functions.tsv")
        self.rel = {}
        p = f"{g}/work/match_fuzzy.tsv"
        if os.path.exists(p):
            for ln in open(p):
                if ln.startswith("#") or not ln.strip():
                    continue
                q = ln.split("\t")
                if float(q[3]) >= 0.7:
                    self.rel[int(q[0], 16)] = (int(q[2], 16), float(q[3]))

    def done(self, a):
        return os.path.exists(f"{self.g}/src/ea_{a:06X}.c")

    def npairs(self, a):
        p = f"{self.g}/work/ram/fn_{a:06X}_regs.tsv"
        return sum(1 for ln in open(p) if "\tpost\t" in ln) if os.path.exists(p) else 0

    def regs_sample(self, a, n=2):
        p = f"{self.g}/work/ram/fn_{a:06X}_regs.tsv"
        return "\n".join(open(p).read().splitlines()[: 1 + 2 * n]) if os.path.exists(p) else ""


DFUNCS = load_funcs("work/functions.tsv")
DSRC = None


def darius_c(d):
    """Text of the verified dariusg C defining the routine at d, if any."""
    global DSRC
    if DSRC is None:
        DSRC = {}
        for f in glob.glob("src/**/*.c", recursive=True):
            t = open(f, errors="ignore").read()
            for m in re.finditer(r"^int (Fn_([0-9A-F]{6}))\(const f3mem_t \*m, f3regs_t \*r\)\n\{.*?^\}", t, re.M | re.S):
                DSRC[int(m.group(2), 16)] = (f, m.group(0))
    return DSRC.get(d)


def prompt(G, a):
    n = G.funcs[a]
    cs, indirect = callees(G.rom, a, n)
    parts, protos = [disasm(G.rom, a, n, f"ROUTINE {a:06X}")], []
    for c in sorted(cs):
        if G.done(c):
            protos.append(f"int Ea_{c:06X}(const f3mem_t *m, f3regs_t *r);  /* ROM 0x{c:06X}, already in ea_auto.h: call it */")
        elif c in G.funcs:
            parts.append(disasm(G.rom, c, G.funcs[c], f"CALLEE {c:06X} (not translated: write it as a static helper in this file)"))
    base = ""
    if a in G.rel:
        d, s = G.rel[a]
        dc = darius_c(d)
        if dc and d in DFUNCS:
            base = (f"\nThis routine is a port of dariusg (Darius Gaiden, same Taito F3 OS) routine 0x{d:06X} "
                    f"(similarity {s:.2f}). Its verified C and its disassembly follow. Port it: keep the "
                    "logic, change every address, A5 offset, constant and callee to the ones in the "
                    "elvactr listing above (the instructions may differ slightly: the elvactr listing wins).\n"
                    f"```\n{disasm(DROM, d, DFUNCS[d], 'DARIUSG ' + format(d, '06X'))}\n```\n"
                    f"dariusg C ({dc[0]}, its helpers are NOT available here: inline them):\n```c\n{dc[1][:7000]}\n```\n")
    return f"""Translate ROM routine 0x{a:06X} of Elevator Action Returns (Taito F3, MC68EC020) to C, register-faithful.

Write ONE C file with exactly this entry point:
    int Ea_{a:06X}(const f3mem_t *m, f3regs_t *r)
- Start the file with:  /* ROM 0x{a:06X}: <one line on what it does> */  then  #include "ea_auto.h"
- r->d[0..7], r->a[0..7] are the 68k registers at entry; r->a[7] = SP points at the return address (stack args at SP+4..). On return leave every register exactly as the ROM leaves it after its rts (a[7] = entry SP + 4, the rts pops the return address). Return 0.
- Memory: rd8/rd16/rd32/wr8/wr16/wr32(m, addr) (big-endian; Work RAM 0x400000-0x41FFFF, ROM < 0x200000 reads the real elvactr ROM, so read tables from ROM with rd*). A5 = 0x408000 (F3_A5); d16(A5) offsets are SIGNED.
- Helpers from ea_auto.h: push_long(m, r, v), push_word(m, r, w), set_word(reg, w).
- A call (jsr/bsr): push_long(m, r, <address of the next instruction>); then call the callee (it pops that return address itself, like rts). Callees:
{chr(10).join(protos) or "(none already translated)"}
- {"The routine has an indirect call (jsr (An) / jsr table): emulate the dispatch for the targets the listing shows; return -1 for a target you do not have." if indirect else "No indirect calls."}
- .b/.w ops change only the low byte/word of a data register. Mind condition codes for Bcc, signed vs unsigned compares, ext/extb sign extension.
- No main(), no printf. Comment each block with its ROM address.

Known pitfalls (from the dariusg work):
{PIT}

Style reference (a verified routine from the dariusg port, same conventions):
```c
{REF}
```
{base}
Captured registers (first calls; columns pair which frame D0-D7 A0-A6 SP):
{G.regs_sample(a)}

Disassembly:
```
{chr(10).join(parts)}
```"""


def ask(msgs, think=None):
    think = THINK if think is None else think
    body = {"model": "qwen3.6-35b-a3b", "messages": msgs, "temperature": 0.2, "max_tokens": 32000,
            "stream": False, "chat_template_kwargs": {"enable_thinking": think},
            "reasoning_effort": "medium" if think else "none"}
    req = urllib.request.Request(URL, json.dumps(body).encode(), {"Content-Type": "application/json"})
    with urllib.request.urlopen(req, timeout=1200) as f:
        out = json.load(f)["choices"][0]["message"]["content"] or ""
    return re.sub(r"<think>.*?</think>\s*", "", out, flags=re.S)


def code_of(reply):
    m = re.findall(r"```(?:c|C)?\n(.*?)```", reply, re.S)
    return max(m, key=len) if m else None


def build_run(G, a, src):
    A = f"{a:06X}"
    exe = f"build/ea1_{A}"
    inc = ["-iquote", "src/auto", "-iquote", f"{G.g}/src"]
    c0 = subprocess.run(["cc", "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror", "-Wno-unused-function", *inc,
                         "-c", src, "-o", "/dev/null"], capture_output=True, text=True)
    if c0.returncode:
        return None, "COMPILE ERROR:\n" + (c0.stderr or c0.stdout)[:3000]
    others = [f for f in glob.glob(f"{G.g}/src/ea_*.c") if not f.endswith(f"ea_{A}.c")]
    c = subprocess.run(["cc", "-std=c11", "-O1", "-w", *inc, f"-DFN1=Ea_{A}", f'-DFN1_ADDR="{A}"',
                        f'-DFN1_DIR="{G.g}/work/ram/fn_"', f'-DGAME_ROM="{G.g}/work/maincpu.bin"',
                        "-o", exe, "tests/test_fn.c", src, *others, "build/libf3.a"], capture_output=True, text=True)
    if c.returncode:
        return None, "COMPILE/LINK ERROR:\n" + (c.stderr or c.stdout)[:3000]
    try:
        with open(f"{exe}.out", "wb") as fo:
            subprocess.run([exe], stdout=fo, stderr=subprocess.STDOUT, timeout=120)
    except subprocess.TimeoutExpired:
        return None, "TIMEOUT: the translation loops forever on the captured input"
    with open(f"{exe}.out", "rb") as fo:
        fo.seek(0, 2); n = fo.tell(); fo.seek(max(0, n - 20000))
        out = fo.read().decode("utf-8", "replace")
    os.remove(f"{exe}.out"); os.remove(exe)
    out = "\n".join(l for l in out.splitlines() if not l.startswith("f3mem:"))
    m = re.search(r"(\d+)/\s*(\d+) calls bit-exact \((\d+) undecompiled", out)
    return ((int(m.group(1)), int(m.group(2)), int(m.group(3))) if m else None), out[-2500:]


def null_passes(G, a):
    p = f"build/ea_null_{a:06X}.c"
    open(p, "w").write(f'#include "ea_auto.h"\nint Ea_{a:06X}(const f3mem_t *m, f3regs_t *r) {{ (void)m; r->a[7] += 4u; return 0; }}\n')
    res, _ = build_run(G, a, p)
    return bool(res and res[1] > 0 and res[0] == res[1])


def accept(G, a, src):
    A = f"{a:06X}"
    dst = f"{G.g}/src/ea_{A}.c"
    if os.path.exists(dst):
        return
    code = open(src).read()
    if f"ROM 0x{A}" not in code:
        code = f"/* ROM 0x{A}: drafted by Strata (scripts/strata_game.py), bit-exact on its capture. */\n" + code
    open(dst, "w").write(code)
    hp = f"{G.g}/src/ea_auto.h"
    h = open(hp).read()
    proto = f"int Ea_{A}(const f3mem_t *m, f3regs_t *r); /* ROM 0x{A} */\n"
    if proto not in h:
        open(hp, "w").write(h.replace("/* END generated */", proto + "/* END generated */"))
    ip = f"{G.g}/tests/fn_auto.inc"
    line = f'    {{ "Ea 0x{A} auto", "{G.g}/work/ram/fn_{A}", Ea_{A}, ALLREGS }},\n'
    if line not in open(ip).read():
        open(ip, "a").write(line)


def log(G, row):
    with open(f"{G.g}/work/strata_results.tsv", "a") as f:
        f.write("\t".join(map(str, row)) + "\n")
    print("\t".join(map(str, row)), flush=True)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("addrs", nargs="*")
    ap.add_argument("--game", default="elvactr")
    ap.add_argument("--max-size", type=int, default=400)
    ap.add_argument("--limit", type=int, default=40)
    ap.add_argument("--retries", type=int, default=1)
    ap.add_argument("--min-pairs", type=int, default=3)
    o = ap.parse_args()
    G = Game(o.game)
    tried = set()
    rp = f"{G.g}/work/strata_results.tsv"
    if os.path.exists(rp):
        tried = {ln.split("\t")[0] for ln in open(rp)}
    if o.addrs:
        todo = [int(x, 16) for x in o.addrs]
    else:
        cand = [a for a, n in G.funcs.items() if n <= o.max_size and f"{a:06X}" not in tried
                and not G.done(a) and G.npairs(a) >= o.min_pairs]
        # fewest untranslated callees first, then dariusg relatives, then size
        def key(a):
            cs, ind = callees(G.rom, a, G.funcs[a])
            return (sum(1 for c in cs if not G.done(c)) + (2 if ind else 0), a not in G.rel, G.funcs[a])
        todo = sorted(cand, key=key)[: o.limit]
    subprocess.run(["make", "-s", "-j16", "build/libf3.a"], check=True)
    for a in todo:
        t0, A, size = time.time(), f"{a:06X}", G.funcs.get(a, 0)
        if G.done(a):
            continue
        if null_passes(G, a):
            log(G, [A, size, "weak", "empty function passes the capture"]); continue
        msgs = [{"role": "system", "content": SYSTEM}, {"role": "user", "content": prompt(G, a)}]
        verdict, detail, nothink = "fail", "", False
        for attempt in range(2 + o.retries):
            if attempt == 1 + o.retries and not nothink:
                break
            try:
                reply = ask(msgs, think=False if nothink else None)
            except Exception as e:
                verdict, detail = "error", repr(e)[:200]; break
            code = code_of(reply)
            src = f"build/ea_draft_{A}.c"
            if not code:
                detail, nothink = "no code block", True; continue
            open(src, "w").write(code)
            res, out = build_run(G, a, src)
            if res and res[1] >= o.min_pairs and res[0] == res[1] and res[2] == 0:
                accept(G, a, src); verdict = "accepted"; detail = f"{res[0]}/{res[1]} try={attempt}"; break
            detail = (f"{res[0]}/{res[1]} undec={res[2]}" if res else (out.strip().splitlines() or ["no output"])[0][:80]) + f" try={attempt}"
            msgs += [{"role": "assistant", "content": "```c\n" + code + "```"},
                     {"role": "user", "content": "The test against the MAME capture says:\n" + out +
                      "\nFix the translation (re-read the instructions for each mismatching address/register) and reply with the whole corrected ```c file."}]
        log(G, [A, size, verdict, detail + f" rel={'%06X' % G.rel[a][0] if a in G.rel else '-'} {int(time.time() - t0)}s"])


if __name__ == "__main__":
    main()
