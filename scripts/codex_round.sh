#!/usr/bin/env bash
# codex_round.sh N : run OpenAI codex (codex exec) on the routines codewhale/agy
# cannot verify, in their own worktrees wt_X1..X3 so it runs beside agy_loop.sh:
#   weak      : captured, but a do-nothing body passes the capture
#   nocapture : no captured call at all (bundle exists)
#   capture   : captured routines codewhale/agy failed or never finished
# 10 per lane, smallest first. Marks them 'codex' in strata_results.tsv so the
# codewhale/agy rounds skip them. Prompt: work/zcap/codex_prompt.txt.
# Logs: work/zcap/codexN_X*.log. Merge afterwards with scripts/codex_merge.sh.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; cd "$ROOT"
N=${1:?round}; PER=${PER:-10}
python3 - "$N" "$PER" <<'PY'
import os,sys
N,per=sys.argv[1],int(sys.argv[2])
res={}
for l in open('work/zcap/strata_results.tsv'):
    p=l.rstrip('\n').split('\t'); res[p[0]]=p[2]
pairs={l.split()[0]:int(l.split()[1]) for l in open('work/zcap/pairs_now.txt')}
rows=[(a,int(s)) for a,s,*_ in (l.split() for l in open('work/zcap/todo.tsv'))]
def kind(a):
    if os.path.exists(f'src/auto/fn_{a}.c') or not os.path.exists(f'work/disasm/bundle_{a.lstrip("0")}.s'): return None
    if res.get(a)=='weak': return 'weak'
    if pairs.get(a,0)==0 and res.get(a) in (None,'fail'): return 'nocapture'
    if pairs.get(a,0)>=1 and res.get(a) in ('fail','agy'): return 'capture'  # left by codewhale/agy
    return None
pool=sorted([(a,s,k) for a,s in rows if (k:=kind(a))],key=lambda x:x[1])
sel=pool[:3*per]; print('pool',len(pool),'taking',len(sel))
for i,w in enumerate(['X1','X2','X3']):
    open(f'work/zcap/codex{N}_{w}.list','w').write(''.join(f'{a}\t{k}\n' for a,s,k in sel[i::3]))
with open('work/zcap/strata_results.tsv','a') as f:
    for a,s,k in sel: f.write(f'{a}\t{s}\tcodex\tassigned to codex, {k} (round {N})\n')
PY
for w in X1 X2 X3; do
  W=/PROCESOS/wt_$w; [ -s work/zcap/codex${N}_$w.list ] || continue
  git worktree remove --force $W 2>/dev/null; git worktree add -q --detach $W HEAD
  mkdir -p $W/work; for f in ram disasm maincpu.bin; do ln -sfn $ROOT/work/$f $W/work/$f; done
  cp work/zcap/pitfalls.txt work/zcap/m68kdis.py $W/work/
  cp work/zcap/codex${N}_$w.list $W/work/batch.list
  (cd $W && make -s -j8 build/test_fn >/dev/null 2>&1)
done
for w in X1 X2 X3; do
  W=/PROCESOS/wt_$w; [ -s work/zcap/codex${N}_$w.list ] || continue
  codex exec -s workspace-write -c model_reasoning_effort="${CODEX_EFFORT:-medium}" -C $W "$(sed "s|WTDIR|$W|" work/zcap/codex_prompt.txt)" > work/zcap/codex${N}_$w.log 2>&1 &
done; wait
echo "codex round $N done"
