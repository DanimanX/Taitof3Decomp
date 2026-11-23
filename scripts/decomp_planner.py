#!/usr/bin/env python3
"""Automatic work piler for the Darius Gaiden object-routine decompilation.

Builds batches of routines that are *ready* (stub + disassembly bundle + MAME
capture, and every `jsr` target already decompiled in `src/`), hands each
batch to a translator backend, and leaves one candidate file per routine in
`work/batches/candidates/<ADDR>.c`.  Verification stays with the parent:

    scripts/verify_batch.sh <ADDR...>      # per-routine test verdicts

Backends
  hermes  hermes -z <prompt> --in ROOT   (local model, no cloud tokens)
  cw      a Codewhale `implement` worker in its own git worktree
          (started by hand from the parent agent; see agent_wt.sh)

Ready-set rules (all of them are why a routine cannot be translated yet):
  * src/enemy/obj_ADDR.c still carries the `(stub)` marker
  * work/disasm/bundle_ADDR.s exists and its `; calls:` header has no
    unresolved target (everything is STOP/already decompiled)
  * work/ram/obj_ADDR_regs.tsv has at least MIN_PAIRS `pre` rows

Usage
  scripts/decomp_planner.py status                 # what is ready / why not
  scripts/decomp_planner.py next --n 6 [--json]    # build the next batch
  scripts/decomp_planner.py run --backend hermes --n 6 [--addrs A1 A2]
  scripts/decomp_planner.py mark A1 A2 --state tried|rejected|accepted
  scripts/decomp_planner.py verify [--all]
"""
import argparse
import glob
import json
import os
import re
import subprocess
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(ROOT)

BATCH_DIR = "work/batches"
CAND_DIR = os.path.join(BATCH_DIR, "candidates")
STATE_FILE = os.path.join(BATCH_DIR, "state.tsv")
MIN_PAIRS = 1


# ---------------------------------------------------------------- discovery
def decompiled_roms():
    """Every ROM address that a src/ file actually implements.

    A real translation annotates itself `/* ROM 0x...... */`. The shipped stub
    says "not decompiled yet". A thin entry point that forwards to an emitted
    helper carries no annotation of its own (obj_11B288.c -> ObjHelper_11B288
    in obj_11B262.c): it is owned by src/enemy/obj_<addr>.c and is not a stub,
    so it counts as decompiled too.
    """
    out = set()
    for p in glob.glob("src/**/*.c", recursive=True):
        try:
            s = open(p, encoding="utf-8", errors="replace").read()
        except OSError:
            continue
        if "not decompiled yet (stub)" in s:
            continue
        out |= {int(x, 16) for x in re.findall(r"ROM 0x([0-9A-Fa-f]{4,6})", s)}
        m = re.search(r"obj_([0-9A-Fa-f]{5,6})\.c$", p)
        if m and re.search(r"\bObj_%s_Tick\s*\(" % m.group(1), s):
            out.add(int(m.group(1), 16))
    return out


def bundle_calls(path):
    """Targets listed in the `; calls:` header line; empty string = none."""
    try:
        first = open(path, encoding="utf-8", errors="replace").read(2000)
    except OSError:
        return None
    m = re.search(r"^; calls:(.*)$", first, re.M)
    if not m:
        return None
    return re.findall(r"([0-9a-fA-F]{6})", m.group(1))


SHARED_HEADER = "src/objects/obj_shared.h"


def shared_roms():
    """Addresses of the primitives promoted to src/objects/obj_shared.c: those
    are available to every routine even though no per-routine file owns them."""
    if not os.path.exists(SHARED_HEADER):
        return set()
    s = open(SHARED_HEADER, encoding="utf-8", errors="replace").read()
    return {int(x, 16) for x in re.findall(r"ROM 0x([0-9A-Fa-f]{4,6})", s)}


_SHARED = None


def shared_rom(addr):
    global _SHARED
    if _SHARED is None:
        _SHARED = shared_roms()
    return int(addr, 16) in _SHARED


def is_routine_target(t):
    """Every name on a bundle's `; calls:` line is a real jsr/bsr/jmp target
    (checked across all 2510 bundles); it also lists already-decompiled ones.
    The address of an `#` immediate in the body is NOT a caller of that
    address, so the body is not consulted here."""
    return True


def pair_count(addr_hex):
    p = f"work/ram/obj_{addr_hex}_regs.tsv"
    if not os.path.exists(p):
        return 0
    with open(p, encoding="utf-8", errors="replace") as f:
        return sum(1 for line in f if "\tpre\t" in line)


def load_state():
    st = {}
    if os.path.exists(STATE_FILE):
        with open(STATE_FILE, encoding="utf-8") as f:
            for line in f:
                parts = line.rstrip("\n").split("\t")
                if len(parts) >= 2:
                    st[parts[0]] = parts[1]
    return st


def save_state(st):
    os.makedirs(BATCH_DIR, exist_ok=True)
    with open(STATE_FILE, "w", encoding="utf-8") as f:
        for k in sorted(st):
            f.write(f"{k}\t{st[k]}\n")


def candidates():
    """[dict] for every stub with a bundle, with the reasons it is not ready."""
    dec = decompiled_roms()
    state = load_state()
    rows = []
    for p in sorted(glob.glob("src/enemy/obj_*.c")):
        try:
            s = open(p, encoding="utf-8", errors="replace").read()
        except OSError:
            continue
        if "not decompiled yet (stub)" not in s:
            continue
        name = os.path.basename(p)[4:-2]
        addr = int(name, 16)
        b = f"work/disasm/bundle_{addr:05X}.s"
        if not os.path.exists(b):
            continue
        calls = bundle_calls(b) or []
        # 0x0D4F26 is not a call: bundles list its blocks as "(via table ...)"
        # after an amortised multiply the shared body already models exactly.
        missing = sorted({c.upper() for c in calls
                          if is_routine_target(c)
                          and int(c, 16) not in dec
                          and not shared_rom(c)
                          and int(c, 16) != 0x0D4F26})
        pairs = pair_count(name)
        reasons = []
        if missing:
            reasons.append("missing callees: " + ",".join(missing))
        if pairs < MIN_PAIRS:
            reasons.append(f"no capture ({pairs} pairs)")
        if state.get(name) in ("tried", "rejected"):
            reasons.append(f"already {state[name]}")
        rows.append(dict(
            addr=addr, name=name, bundle=b, size=os.path.getsize(b),
            missing=missing, pairs=pairs, state=state.get(name, ""),
            ready=not reasons, reasons=reasons,
        ))
    return rows


def ready_batch(n, rows=None):
    rows = rows if rows is not None else candidates()
    ready = [r for r in rows if r["ready"]]
    # smallest bundle first: the routine whose translation is closest to a
    # single basic block, which is what a local model gets right
    ready.sort(key=lambda r: (r["size"], r["addr"]))
    return ready[:n]


# ---------------------------------------------------------------- prompting
PROMPT_HEAD = """Task: translate already-stubbed Taito F3 (68EC020) object tick routines into register-faithful C. Write ONLY the files named below.
Scope: your batch's src/enemy/obj_ADDR.c files in this worktree; read-only for everything else.
Do: for each ADDR
  1. read work/disasm/bundle_ADDR.s (flow-following disassembly of ROM 0xADDR and its callees; already-decompiled routines appear as STOP) and work/ram/obj_ADDR_regs.tsv (A0 per tick);
  2. replace the stub in src/enemy/obj_ADDR.c with int Obj_ADDR_Tick(const f3mem_t *m, uint32_t obj);
  3. `make build/test_objects && F3_ONLY=ADDR ./build/test_objects` until it prints N/ N ticks bit-exact.
Rules: exact 68k semantics; call already-decompiled callees through their headers (match the `ROM 0x......` comment); other bundle blocks become static helpers with a `/* ROM 0x...... */` comment; if a path has an unresolvable indirect call, return -1 BEFORE any side effect of that path.
Read work/worker_prompt.md for the full style guide and the verified pitfalls. Do not edit anything outside your batch. Do not commit. Final reply: one line per ADDR: `ADDR N/N` plus one short reason for failures.

Batch:
"""
PROMPT_TAIL = """
"""


def build_prompt(addrs):
    os.makedirs(BATCH_DIR, exist_ok=True)
    with open(os.path.join(BATCH_DIR, "worker_prompt.md"), "w", encoding="utf-8") as f:
        src = "work/drafts/worker_prompt.md"
        f.write(open(src, encoding="utf-8").read() if os.path.exists(src) else "")
    return PROMPT_HEAD + " ".join(addrs) + PROMPT_TAIL


# ---------------------------------------------------------------- hermes
HERMES_PROMPT = """Task: for each routine below, read work/disasm/bundle_ADDR.s and print the C translation. Do NOT call the bash tool; do NOT create, edit, or delete any file (a second writer would collide with this workspace).
Scope: read-only. Inputs: work/disasm/bundle_ADDR.s, work/ram/obj_ADDR_regs.tsv, src/common/f3mem.h, src/objects/obj_pool.h, src/objects/obj_common.h, src/video/sprite.h, src/enemy/obj_ADDR.h, plus the header of any routine named on the bundle's "; calls:" line.
Do: for each ADDR, print one block shaped exactly like

=== FILE src/enemy/obj_ADDR.c ===
#include "obj_ADDR.h"
... the whole file ...
=== END ===

Body rules:
- int Obj_ADDR_Tick(const f3mem_t *m, uint32_t obj);  /* obj = A0, A5 = F3_A5 (0x408000) */
- exact 68k sizes: clr.w -> wr16(...,0), move.b -> wr8, add.w is 16-bit with wrap, .w data/address values sign-extended where the CPU does it, every (An)+ advances the pointer.
- ROM tables: rd16/rd32 at the absolute address (the listing already prints it).
- a callee the bundle lists as STOP/already decompiled MUST be called through its header function, never re-implemented.
- every other block in the bundle (FUN_/LAB_/a second OBJ_ entry) becomes a static helper in the same file, preceded by /* ROM 0xXXXXXX */; call it. If the only way through a block is an indirect call you cannot resolve from the bundle or src/, return -1 there before any side effect and say so in your summary.
- no prose and no markdown inside the file block.

After the blocks, print exactly one line per ADDR: `ADDR moved=name1,name2` naming every branch target whose function was emitted as a helper (use its real ROM address); write `moved=` empty when none.
Then stop.

Routines:
"""


def parse_files(text):
    """{addr: c_source} from `=== FILE path ===` ... `=== END ===` blocks."""
    out = {}
    for m in re.finditer(r"=== FILE (\S+) ===\s*\n(.*?)\n=== END ===", text, re.S):
        path, body = m.group(1), m.group(2)
        mm = re.search(r"obj_([0-9A-Fa-f]+)\.c$", path)
        if mm:
            out[mm.group(1).upper()] = (mm.group(1).upper(), body.rstrip() + "\n")
    return out


def parse_moved(text):
    mv = {}
    for m in re.finditer(r"^\s*([0-9A-Fa-f]{5,6})\s+moved=([0-9A-Fa-f,]*)\s*$", text, re.M):
        mv[m.group(1).upper()] = [x.upper() for x in m.group(2).split(",") if x]
    return mv


def hermes_extract(addrs, worktree):
    os.makedirs(CAND_DIR, exist_ok=True)
    prompt = HERMES_PROMPT + "\n".join(addrs) + "\n"
    stamp = time.strftime("%H%M%S")
    log = os.path.join(BATCH_DIR, f"hermes_{stamp}.log")
    with open(log, "w", encoding="utf-8") as fh:
        fh.write(prompt)
    try:
        r = subprocess.run(
            ["hermes", "-z", prompt, "--in", os.path.abspath(worktree),
             "-m", "qwen3.6-35b-a3b-ud-q4_k_xl"],
            capture_output=True, text=True, timeout=int(os.environ.get("F3_HERMES_TIMEOUT", "900")))
        text = r.stdout
    except subprocess.TimeoutExpired as e:
        text = (e.stdout or b"").decode(errors="replace") if isinstance(e.stdout, bytes) else (e.stdout or "")
        print(f"hermes: TIMEOUT on {' '.join(addrs)}", file=sys.stderr)
    with open(log, "a", encoding="utf-8") as fh:
        fh.write("\n\n----- REPLY -----\n" + text)
    files = parse_files(text)
    moved = parse_moved(text)
    os.makedirs(os.path.join(CAND_DIR, "moved"), exist_ok=True)
    for a in addrs:
        if a in files:
            name, body = files[a]
            with open(os.path.join(CAND_DIR, f"{name}.c"), "w", encoding="utf-8") as f:
                f.write(body)
            with open(os.path.join(CAND_DIR, "moved", f"{a}.txt"), "w", encoding="utf-8") as f:
                f.write(",".join(moved.get(a, [])))
            print(f"hermes: {a} -> {CAND_DIR}/{a}.c")
        else:
            print(f"hermes: {a} -> no file block", file=sys.stderr)
    return log


# ---------------------------------------------------------------- verify
TEST_LINE = re.compile(r"Obj 0x0*([0-9A-Fa-f]+)\s+(\d+)/\s*(\d+) ticks bit-exact \(([^)]*)\)")


def install_candidate(addr, cand_path):
    dst_c = f"src/enemy/obj_{addr}.c"
    dst_h = f"src/enemy/obj_{addr}.h"
    if not os.path.exists(dst_h):
        with open(dst_h, "w", encoding="utf-8") as f:
            f.write(f'#ifndef TAITOF3_OBJ_{addr}_H\n#define TAITOF3_OBJ_{addr}_H\n'
                    f'#include "../common/f3mem.h"\nint Obj_{addr}_Tick(const f3mem_t *m, uint32_t obj);'
                    f' /* ROM 0x{addr} */\n#endif\n')
    src = open(cand_path, encoding="utf-8").read()
    # the ROM annotation is what marks a routine decompiled (scripts/coverage.py)
    if not re.search(r"ROM 0x0*" + addr.lstrip("0"), src):
        src = src.replace("#include", f"/* ROM 0x{addr} */\n#include", 1)
    with open(dst_c, "w", encoding="utf-8") as f:
        f.write(src)


def verify_one(addr):
    """Build once, run one routine. Returns (state, ok, tested, extra, msg)."""
    r = subprocess.run(["make", "-s", "build/test_objects"], capture_output=True, text=True)
    if r.returncode != 0:
        errs = [l for l in r.stderr.splitlines() if "error" in l][:3]
        return "build-fail", 0, 0, "", " | ".join(errs)
    env = dict(os.environ, F3_ONLY=addr)
    r = subprocess.run(["./build/test_objects"], capture_output=True, text=True, env=env, timeout=600)
    out = r.stdout + r.stderr
    for m in TEST_LINE.finditer(out):
        if m.group(1).upper() != addr.upper():
            continue
        ok, tested, extra = int(m.group(2)), int(m.group(3)), m.group(4)
        if tested > 0 and ok == tested and "0 reach undecompiled" in extra:
            return "PASS", ok, tested, extra, ""
        return "FAIL", ok, tested, extra, ""
    return "no-test", 0, 0, "", out.strip().splitlines()[-1] if out.strip() else "no output"


def main():
    ap = argparse.ArgumentParser()
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("status")
    p_next = sub.add_parser("next")
    p_next.add_argument("--n", type=int, default=6)
    p_next.add_argument("--json", action="store_true")
    p_run = sub.add_parser("run")
    p_run.add_argument("--backend", default="hermes", choices=["hermes", "print"])
    p_run.add_argument("--n", type=int, default=6)
    p_run.add_argument("--addrs", nargs="*", default=None)
    p_run.add_argument("--worktree", default=ROOT)
    p_mark = sub.add_parser("mark")
    p_mark.add_argument("addrs", nargs="+")
    p_mark.add_argument("--state", required=True, choices=["tried", "rejected", "accepted"])
    p_ver = sub.add_parser("verify")
    p_ver.add_argument("--all", action="store_true")
    p_ver.add_argument("--addrs", nargs="*", default=None)
    a = ap.parse_args()

    if a.cmd == "status":
        rows = candidates()
        dec = len(decompiled_roms())
        stubs = [p for p in glob.glob("src/enemy/obj_*.c")
                 if "not decompiled yet (stub)" in open(p, errors="replace").read()]
        print(f"stubs={len(stubs)} decompiled_ROMs={dec} stubs_with_bundle={len(rows)} "
              f"ready={len([r for r in rows if r['ready']])}")
        seen, out = set(), {}
        for r in rows:
            if r["ready"]:
                continue
            key = tuple(r["reasons"])
            out[key] = out.get(key, 0) + 1
        for key, n in sorted(out.items(), key=lambda kv: -kv[1]):
            print(f"  {n:4d}  {'; '.join(key)}")
        return

    if a.cmd == "next":
        b = ready_batch(a.n)
        if a.json:
            print(json.dumps(b, indent=1))
        else:
            for i in range(0, len(b), max(1, a.n // 3 if a.n >= 3 else a.n)):
                chunk = [r["name"] for r in b[i:i + max(1, a.n // 3 if a.n >= 3 else a.n)]]
                print(" ".join(chunk))
        return

    if a.cmd == "run":
        addrs = a.addrs or [r["name"] for r in ready_batch(a.n)]
        if not addrs:
            print("nothing ready", file=sys.stderr)
            return 1
        if a.backend == "print":
            print(build_prompt(addrs))
            return 0
        hermes_extract(addrs, a.worktree)
        return 0

    if a.cmd == "mark":
        st = load_state()
        for ad in a.addrs:
            st[ad.upper()] = a.state
        save_state(st)
        return 0

    if a.cmd == "verify":
        st = load_state()
        addrs = a.addrs or (sorted(st) if a.all else
                            [os.path.basename(p)[:-2] for p in glob.glob(os.path.join(CAND_DIR, "*.c"))])
        verdicts = {}
        for ad in addrs:
            cand = os.path.join(CAND_DIR, f"{ad}.c")
            if not os.path.exists(cand):
                continue
            mv = os.path.join(CAND_DIR, "moved", f"{ad}.txt")
            moved = open(mv).read().split(",") if os.path.exists(mv) else []
            install_candidate(ad, cand)
            moved = [x for x in moved if x]
            if moved:
                msg = ""
                for t in moved:
                    if not os.path.exists(f"work/disasm/bundle_{int(t, 16):05X}.s"):
                        msg = f"branch target 0x{t} has no bundle: run scripts/export_bundle.sh {t}"
                        break
                verdicts[ad] = ("needs-helper", 0, 0, ",".join(moved), msg)
                print(f"{ad} NEEDS-HELPER ({','.join(moved)}) {msg}")
                continue
            state, ok, tested, extra, msg = verify_one(ad)
            verdicts[ad] = (state, ok, tested, extra, msg)
            print(f"{ad} {state} {ok}/{tested} {extra} {msg}".rstrip())
        savefile = os.path.join(BATCH_DIR, "verdicts.json")
        with open(savefile, "w", encoding="utf-8") as f:
            json.dump(verdicts, f, indent=1)
        return 0


if __name__ == "__main__":
    sys.exit(main())
