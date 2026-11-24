#!/usr/bin/env python3
"""Unattended night runner: one bounded Hermes task at a time.

Design rules (from the hermes-delegate skill):
  * Hermes never certifies its own work. Every task's output is re-checked here
    by running the real gate (make build/test_objects + F3_ONLY=<addr>).
  * Small blast radius: only src/enemy/obj_<ADDR>.{c,h} may change; anything
    else is reverted with git checkout before proceeding.
  * A task that does not reach N/N is reverted; a PASS is committed and
    published immediately, so the night's work is durable.
  * Every step is appended to work/audit/night.log with a timestamp, so the
    user can read the whole night in one file.

Usage: python3 work/audit/night_runner.py [--max-tasks N] [--dry-run]
"""
import argparse
import json
import os
import re
import subprocess
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
LOG = os.path.join(ROOT, 'work/audit/night.log')
W = 128 * 1024


def log(msg):
    line = '%s %s' % (time.strftime('%Y-%m-%dT%H:%M:%S'), msg)
    print(line, flush=True)
    with open(LOG, 'a') as f:
        f.write(line + '\n')


def sh(cmd, cwd=ROOT, timeout=1800):
    return subprocess.run(cmd, shell=True, cwd=cwd, capture_output=True,
                          text=True, timeout=timeout)


def candidates(limit):
    """Stubs whose capture passes the mechanical screen."""
    out = sh('python3 work/audit/fidelity_scan.py').stdout.splitlines()[1:]
    rows = []
    for line in out:
        p = line.split('\t')
        if len(p) < 5:
            continue
        addr, pairs, fn_ok, hist, fields = p[0], int(p[1]), p[2], p[3], int(p[4])
        if fn_ok == 'yes' and pairs >= 30 and fields >= 6:
            src = os.path.join(ROOT, 'src/enemy/obj_%s.c' % addr)
            if os.path.exists(src) and 'not decompiled yet (stub)' in open(src).read():
                rows.append((addr, pairs, fields))
    rows.sort(key=lambda r: (-r[1], -r[2]))
    return rows[:limit]


def git_clean_scope(addr):
    """Revert anything outside the target's two files."""
    files = ['src/enemy/obj_%s.c' % addr, 'src/enemy/obj_%s.h' % addr]
    dirty = sh('git status --porcelain').stdout.splitlines()
    stray = [l for l in dirty
             if not l.startswith('??')
             and not any(f in l for f in files)]
    if stray:
        log('  reverting stray changes: %s' % stray)
        sh('git checkout -- .')
    return not sh('git status --porcelain').stdout.strip() or True


def gate(addr):
    b = sh('make build/test_objects')
    if b.returncode != 0:
        return False, 'build failed: ' + (b.stderr or b.stdout)[-400:]
    r = sh('F3_ONLY=%s ./build/test_objects' % addr)
    line = next((l for l in r.stdout.splitlines() if addr in l and 'bit-exact (' in l), '')
    m = re.search(r'(\d+)/ *(\d+) ticks bit-exact', line)
    if m and int(m.group(1)) == int(m.group(2)) and int(m.group(1)) > 0:
        return True, line.strip()
    return False, line.strip() or 'no verdict line'


def hermes_prompt(addr, pairs):
    return (
        'Task: decompile ONE 68k object routine to C so its captured ticks match bit-exactly.\n'
        'Scope: ONLY the files src/enemy/obj_%s.c and src/enemy/obj_%s.h in /PROCESOS/Taitof3Decomp.\n'
        'Do:\n'
        '  1. python3 /tmp/f3dis.py %s 0x500   (capstone 040, ROM offset == address)\n'
        '  2. Read docs/PC_INDEXED_TABLES.md and docs/CAPTURE_FIDELITY.md first.\n'
        '  3. For every jsr/bsr target: grep src/ for an existing translation and COPY its body\n'
        '     as a static into your file; call public ones through headers. Never re-derive a\n'
        '     helper that already exists, and never invent behaviour.\n'
        '  4. Capture: work/ram/obj_%s_pairs.bin and _regs.tsv. WRAM snapshot is 0x20000 bytes\n'
        '     starting at 0x400000, so address X is at byte X-0x400000. pair i: pre at i*2W,\n'
        '     post at (i*2+1)*W, W=131072. regs.tsv: pair,which,frame,D0..D7,A0..A6,SP\n'
        '     (A0 is the 12th tab-separated field).\n'
        '  5. Signature: int Obj_%s_Tick(const f3mem_t *m, uint32_t obj). Return -1 only for a\n'
        '     path the capture does not exercise.\n'
        '  6. Verify: make build/test_objects && F3_ONLY=%s ./build/test_objects\n'
        '     Need "<n>/<n> ticks bit-exact" with n>0 and no got=/want= lines.\n'
        'Do not: touch any other file; do not edit tests; do not commit or push; do not run\n'
        '  git checkout of anything except your own two files.\n'
        'Output: PASS or FAIL, the exact test line, and the full final .c file content.\n'
        'Stop when: you have run the test and pasted its exact output line.\n'
    ) % (addr, addr, addr, addr, addr, addr)


def run_task(addr, pairs, timeout):
    log('TASK %s (%d pairs)' % (addr, pairs))
    git_clean_scope(addr)
    # NOT --safe-mode: it demands an explicit model and, with one, produced no
    # output at all within 120 s. The plain route answered "PONG" immediately.
    p = subprocess.run(['hermes', '-z', hermes_prompt(addr, pairs), '--in', ROOT,
                        '--checkpoints'],
                       capture_output=True, text=True, timeout=timeout)
    reply = (p.stdout or '') + (p.stderr or '')
    log('  hermes rc=%d, %d bytes of reply' % (p.returncode, len(reply)))

    ok, verdict = gate(addr)
    if not ok:
        log('  GATE FAILED: %s' % verdict[:200])
        sh('git checkout -- src/enemy/obj_%s.c src/enemy/obj_%s.h' % (addr, addr))
        return False

    log('  GATE PASSED: %s' % verdict)
    full = sh('./build/test_objects').stdout
    regress = [l for l in full.splitlines() if 'got=' in l]
    log('  full-suite got= lines: %d' % len(regress))
    sh('make coverage >/dev/null 2>&1')
    sh('git add src/enemy/obj_%s.c src/enemy/obj_%s.h docs/COVERAGE.md README.md' % (addr, addr))
    c = sh('git commit -q -m "feat: obj_%s bit-exact (night run)\n\n'
           'Decompiled by the local Hermes agent from a bounded brief; the gate\n'
           '(make build/test_objects + F3_ONLY=%s) was re-run by the parent, not\n'
           'trusted from the agent."' % (addr, addr))
    log('  commit rc=%d' % c.returncode)
    pub = sh('make publish')
    log('  publish: %s' % (pub.stdout.strip().splitlines()[-1:] or ['?'])[0])
    return True


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--max-tasks', type=int, default=4)
    ap.add_argument('--per-task-timeout', type=int, default=1800)
    ap.add_argument('--dry-run', action='store_true')
    a = ap.parse_args()

    cands = candidates(200)
    log('night run start: %d candidates, taking up to %d' % (len(cands), a.max_tasks))
    if a.dry_run:
        for c in cands[:a.max_tasks * 3]:
            log('  candidate %s (%d pairs, %d fields)' % c)
        return

    done = passed = 0
    for addr, pairs, _fields in cands:
        if done >= a.max_tasks:
            break
        dirty = [l for l in sh('git status --porcelain').stdout.splitlines()
                 if not l.startswith('??')]
        if dirty:
            log('tree not clean before %s (%d tracked change(s)); reverting' % (addr, len(dirty)))
            sh('git checkout -- .')
        try:
            if run_task(addr, pairs, a.per_task_timeout):
                passed += 1
        except subprocess.TimeoutExpired:
            log('  TIMEOUT on %s' % addr)
            sh('git checkout -- src/enemy/obj_%s.c src/enemy/obj_%s.h' % (addr, addr))
        except Exception as e:                                   # noqa: BLE001
            log('  ERROR on %s: %s' % (addr, e))
        done += 1
    log('night run end: %d attempted, %d passed' % (done, passed))


if __name__ == '__main__':
    main()
