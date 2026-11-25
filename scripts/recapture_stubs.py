#!/usr/bin/env python3
"""Mass MAME recapture for unverified object stubs using survey savestates.

Finds all `src/enemy/obj_*.c` files still containing `not decompiled yet (stub)`
(or only `ready` stubs from decomp_planner.py with --ready-only), matches each
address to the closest savestate in `states/dariusg/` via `work/survey/*.tsv`,
runs `scripts/capture_fn.sh` with `F3_SPR=1 F3_GFX=1`, normalizes `obj_<ADDR>_*`
filenames to match `tests/test_objects.c`, and keeps the new capture only if
`pairs > 0` and `work/audit/precheck.py <ADDR>` passes.
"""
import argparse
import glob
import json
import os
import re
import shutil
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
STATES_DIR = ROOT / "states" / "dariusg"
RAM_DIR = ROOT / "work" / "ram"
SURVEY_DIR = ROOT / "work" / "survey"
PRECHECK = ROOT / "work" / "audit" / "precheck.py"
CAPTURE_FN = ROOT / "scripts" / "capture_fn.sh"
LOG_OK = ROOT / "work" / "recapture_ok.txt"
LOG_SKIP = ROOT / "work" / "recapture_skip.txt"

PFX_MAP = {
    "long_A": "LA_",
    "long_B": "LB_",
    "long_C": "LC_",
    "long_D": "LD_",
    "long_E": "LE_",
    "todo4": "run1_",
    "todo4b": "run1_",
    "run1": "run1_",
}


def load_test_names():
    """Map stripped uppercase hex addr -> exact suffix used in test_objects.c."""
    txt = (ROOT / "tests" / "test_objects.c").read_text()
    mapping = {}
    for m in re.finditer(r'"work/ram/obj_([0-9A-Fa-f]+)"', txt):
        name = m.group(1).upper()
        mapping[name.lstrip("0")] = name
    return mapping


def find_stubs(ready_only=False):
    test_names = load_test_names()
    stubs = set()
    for p in (ROOT / "src" / "enemy").glob("obj_*.c"):
        if "not decompiled yet (stub)" in p.read_text():
            addr = p.stem[4:].upper()
            stubs.add(addr)

    if ready_only:
        out = subprocess.check_output(
            ["python3", str(ROOT / "scripts" / "decomp_planner.py"), "next", "--n", "300", "--json"],
            text=True,
        )
        ready = {format(x["addr"], "X") for x in json.loads(out)}
        stubs = {a for a in stubs if a.lstrip("0") in {r.lstrip("0") for r in ready}}

    return sorted(stubs, key=lambda a: int(a, 16)), test_names


def load_savestates():
    if not STATES_DIR.exists():
        return set(), {}
    stas = {p.stem for p in STATES_DIR.glob("*.sta")}
    by_pfx = {}
    for s in stas:
        m = re.match(r"^([A-Za-z0-9]+_)(\d{6})$", s)
        if m:
            by_pfx.setdefault(m.group(1), []).append((int(m.group(2)), s))
    for k in by_pfx:
        by_pfx[k].sort()
    return stas, by_pfx


def find_best_capture_plan(stubs, stas, by_pfx):
    """Return {stripped_addr: (delta, state_name, group)} picking smallest >=0 frame delta."""
    tsvs = sorted(
        glob.glob(str(SURVEY_DIR / "long_*.tsv"))
        + glob.glob(str(SURVEY_DIR / "*_inv.tsv"))
        + glob.glob(str(SURVEY_DIR / "todo4*.tsv"))
        + glob.glob(str(SURVEY_DIR / "run1.tsv"))
    )
    want = {a.lstrip("0") for a in stubs}
    best = {}

    for tsv in tsvs:
        base = os.path.basename(tsv).replace("_inv.tsv", "").replace(".tsv", "")
        pfx = PFX_MAP.get(base, "")
        with open(tsv) as fh:
            for line in fh:
                cols = line.strip().split("\t")
                if len(cols) < 2:
                    continue
                key = cols[0].upper().lstrip("0")
                if key not in want:
                    continue
                grp = int(cols[3]) if len(cols) > 3 and cols[3].isdigit() else 0
                if cols[1].isdigit():
                    frame = int(cols[1])
                    if pfx and pfx in by_pfx:
                        cands = [(frame - sf, st) for sf, st in by_pfx[pfx] if 0 <= (frame - sf) <= 4500]
                        if cands:
                            delta, st = min(cands)
                            if key not in best or delta < best[key][0]:
                                best[key] = (delta, st, grp)
                    elif base in stas:
                        delta = frame
                        if delta <= 30000 and (key not in best or delta < best[key][0]):
                            best[key] = (delta, base, grp)
                elif cols[1] in stas:
                    delta = int(cols[2]) if len(cols) > 2 and cols[2].isdigit() else 5000
                    if key not in best or delta < best[key][0]:
                        best[key] = (delta, cols[1], grp)
    return best


def backup_existing(ram_name):
    bak = {}
    for ext in ("pairs.bin", "regs.tsv", "spr.bin", "gfx.bin"):
        p = RAM_DIR / f"obj_{ram_name}_{ext}"
        if p.exists():
            dst = RAM_DIR / f"obj_{ram_name}_{ext}.recap_bak"
            shutil.copy2(p, dst)
            bak[ext] = (p, dst)
    return bak


def restore_backup(bak):
    for p, dst in bak.values():
        shutil.move(str(dst), str(p))


def drop_backup(bak):
    for _, dst in bak.values():
        dst.unlink(missing_ok=True)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--ready-only", action="store_true", help="Only recapture planner-ready stubs")
    ap.add_argument("--limit", type=int, default=0, help="Max stubs to capture (0 = all)")
    ap.add_argument("--samples", type=int, default=60, help="Samples per stub (default 60)")
    ap.add_argument("--dry-run", action="store_true", help="Print capture plan without running MAME")
    ap.add_argument("--resume", action="store_true", help="Skip addresses already in work/recapture_ok.txt")
    ap.add_argument("--only-long", action="store_true", help="Only use L[A-E]_* savestates")
    args = ap.parse_args()

    stubs, test_names = find_stubs(ready_only=args.ready_only)
    stas, by_pfx = load_savestates()
    plan = find_best_capture_plan(stubs, stas, by_pfx)

    done_ok = set()
    if args.resume:
        for lf in (LOG_OK, LOG_SKIP):
            if lf.exists():
                for ln in lf.read_text().splitlines():
                    if ln.strip():
                        done_ok.add(ln.split()[0].upper().lstrip("0"))

    ordered = []
    for addr in stubs:
        k = addr.lstrip("0")
        if k in plan and k not in done_ok:
            delta, st, grp = plan[k]
            if args.only_long and not re.match(r"^L[ABCDE]_", st):
                continue
            ram_name = test_names.get(k, addr)
            ordered.append((delta, addr, ram_name, st, grp))
    ordered.sort()

    if args.limit > 0:
        ordered = ordered[: args.limit]

    print(f"stubs={len(stubs)} matched_with_savestate={len(plan)} to_run={len(ordered)}")
    if args.dry_run:
        for delta, addr, ram_name, st, grp in ordered:
            print(f"  {ram_name:8s} state={st:16s} delta={delta:5d} grp={grp}")
        return 0

    ok_count = 0
    for idx, (delta, addr, ram_name, st, grp) in enumerate(ordered, 1):
        fn_hex = addr.lower().lstrip("0") or "0"
        en, ex = ("10568", "1056A") if grp == 11 else ("104EE", "104F0")
        maxf = min(max(delta + 1800, 2500), 12000)
        env = os.environ.copy()
        env["F3_SPR"] = "1"
        env["F3_GFX"] = "1"
        env["F3_SAMPLES"] = str(args.samples)
        env["F3_COND"] = f"a1=={fn_hex}"
        env["F3_MAXFRAMES"] = str(maxf)
        if re.search(r"_\d{6}$", st):
            env["F3_NOFFSET"] = str(int(st.rsplit("_", 1)[1]))
        else:
            env.pop("F3_NOFFSET", None)

        bak = backup_existing(ram_name)
        try_prefix = RAM_DIR / f"obj_{ram_name}_recap"
        t0 = time.time()
        res = subprocess.run(
            ["timeout", "-s", "KILL", "360", str(CAPTURE_FN), en, ex, str(args.samples), str(try_prefix), st],
            env=env,
            capture_output=True,
            text=True,
        )
        m = re.search(r"pairs=(\d+)", res.stdout + res.stderr)
        pairs = int(m.group(1)) if m else 0
        dt = time.time() - t0

        passed = False
        if pairs > 0:
            for ext in ("pairs.bin", "regs.tsv", "spr.bin", "gfx.bin"):
                src = Path(f"{try_prefix}_{ext}")
                if src.exists():
                    shutil.move(str(src), str(RAM_DIR / f"obj_{ram_name}_{ext}"))
            chk = subprocess.run(["python3", str(PRECHECK), ram_name], capture_output=True, text=True)
            passed = chk.returncode == 0

        for ext in ("pairs.bin", "regs.tsv", "spr.bin", "gfx.bin"):
            Path(f"{try_prefix}_{ext}").unlink(missing_ok=True)

        if passed:
            drop_backup(bak)
            ok_count += 1
            with open(LOG_OK, "a") as lf:
                lf.write(f"{ram_name}\t{st}\tpairs={pairs}\n")
            print(f"[{idx}/{len(ordered)}] PASS {ram_name} state={st} pairs={pairs} ({dt:.1f}s)", flush=True)
        else:
            restore_backup(bak)
            with open(LOG_SKIP, "a") as sf:
                sf.write(f"{ram_name}\t{st}\tpairs={pairs}\n")
            print(f"[{idx}/{len(ordered)}] SKIP {ram_name} state={st} pairs={pairs} ({dt:.1f}s)", flush=True)

    print(f"Done: {ok_count}/{len(ordered)} freshly captured & precheck-verified.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
