#!/usr/bin/env python3
"""Check (and optionally accept) a hand/agent draft of one elvactr routine.

  scripts/check_ea.py listing <ADDR>          print the disassembly + callee listings + Darius relative
  scripts/check_ea.py check <ADDR> <draft.c>  build against the capture, report bit-exact calls
  scripts/check_ea.py accept <ADDR> <draft.c> check, refuse if not 100% or if an empty function passes,
                                              then install as <game>/src/ea_<ADDR>.c
The draft defines int Ea_<ADDR>(const f3mem_t *m, f3regs_t *r) and includes "ea_auto.h".
Same rules as scripts/strata_game.py (which this reuses).
"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
os.chdir(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import strata_game as sg

cmd, a = sys.argv[1], int(sys.argv[2], 16)
G = sg.Game(os.environ.get("GAME", "elvactr"))
if cmd == "listing":
    print(sg.prompt(G, a))
    sys.exit(0)
src = sys.argv[3]
res, out = sg.build_run(G, a, src)
print(out[-1500:])
ok = bool(res and res[1] > 0 and res[0] == res[1])
print(f"RESULT {a:06X}: {res} {'BIT-EXACT' if ok else 'MISMATCH'}")
if cmd == "accept":
    if not ok:
        sys.exit(1)
    if sg.null_passes(G, a):
        print("REFUSED: an empty function also passes this capture (weak capture)")
        sys.exit(2)
    sg.accept(G, a, src)
    print(f"ACCEPTED {G.g}/src/ea_{a:06X}.c")
sys.exit(0 if ok else 1)
