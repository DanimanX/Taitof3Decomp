#!/usr/bin/env python3
"""Nearest dariusg function for every function of <game>, by Jaccard
similarity of normalised instruction trigrams (tokens as in match_funcs.py).
Output <game>/work/match_fuzzy.tsv: game_addr size dariusg_addr score name
(score >= 0.5 only). Usage: scripts/match_fuzzy.py <game>"""
import sys, os
sys.argv += [] ; ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, f"{ROOT}/scripts")
game = sys.argv[1]
import importlib.util
spec = importlib.util.spec_from_file_location("mf", f"{ROOT}/scripts/match_funcs.py")
# reuse load()/tokens() without running the script body
src = open(f"{ROOT}/scripts/match_funcs.py").read().split("drom = open")[0]
ns = {"__file__": f"{ROOT}/scripts/match_funcs.py"}
sys_argv = sys.argv; sys.argv = [sys.argv[0], game]; exec(src, ns); sys.argv = sys_argv
load, tokens = ns["load"], ns["tokens"]
drom = open(f"{ROOT}/work/maincpu.bin", "rb").read()
grom = open(f"{ROOT}/{game}/work/maincpu.bin", "rb").read()
dfs, gfs = load(f"{ROOT}/work/functions.tsv"), load(f"{ROOT}/{game}/work/functions.tsv")
def grams(t): return {t[i:i + 3] for i in range(max(1, len(t) - 2))}
dg, inv = {}, {}
for a, (n, _) in dfs.items():
    if n < 8: continue
    g = grams(tokens(drom, a, n)); dg[a] = g
    for x in g: inv.setdefault(x, []).append(a)
rows, tot, hit = [], 0, {0.9: 0, 0.7: 0, 0.5: 0}
for a, (n, _) in sorted(gfs.items()):
    tot += n
    if n < 8: continue
    g = grams(tokens(grom, a, n)); cnt = {}
    for x in g:
        lst = inv.get(x, ())
        if len(lst) > 300: continue          # very common trigram: no signal
        for d in lst: cnt[d] = cnt.get(d, 0) + 1
    best, bs = None, 0.0
    for d, c in cnt.items():
        s = c / (len(g) + len(dg[d]) - c)
        if s > bs: best, bs = d, s
    if best is not None and bs >= 0.5:
        rows.append(f"{a:06X}\t{n}\t{best:06X}\t{bs:.2f}\t{dfs[best][1]}")
        for k in hit:
            if bs >= k: hit[k] += n
open(f"{ROOT}/{game}/work/match_fuzzy.tsv", "w").write(
    "# game_addr\tsize\tdariusg_addr\tscore\tdariusg_name\n" + "\n".join(rows) + "\n")
print(f"{game}: {tot} code bytes; with a dariusg relative at >=0.9: {hit[0.9]/tot:.1%}, "
      f">=0.7: {hit[0.7]/tot:.1%}, >=0.5: {hit[0.5]/tot:.1%}")
