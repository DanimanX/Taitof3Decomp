#!/usr/bin/env bash
# agy_round.sh N [MINSIZE] [MAXSIZE] : pick the next 30 untried captured routines
# (largest first within the size range), reset wt_G1..G3 at HEAD, and run three
# self-verifying agy (Gemini 4 Hydrogen) batches of 10. Logs: work/zcap/agyN_G*.log
# ENGINE=cw runs codewhale (deepseek-flash) instead of agy; MINPAIRS (default 3)
# is the minimum captured calls; ORDER=small takes smallest first.
# Model: $AGY_MODEL (default gemini-4-hydrogen-medium; -high for hard rounds).
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; cd "$ROOT"
N=$1 LO=${2:-1} HI=${3:-300}
ENGINE=${ENGINE:-agy}
python3 - "$N" "$LO" "$HI" "${MINPAIRS:-3}" "${ORDER:-large}" "$ENGINE" <<'PY'
import os,sys
N,lo,hi,mp,order,eng=sys.argv[1],int(sys.argv[2]),int(sys.argv[3]),int(sys.argv[4]),sys.argv[5],sys.argv[6]
res={}
for l in open('work/zcap/strata_results.tsv'):
    p=l.rstrip('\n').split('\t'); res[p[0]]=p[2]
bad=set()  # former codewhale skips, now handed to agy
pairs={l.split()[0]:int(l.split()[1]) for l in open('work/zcap/pairs_now.txt')}
rows=[(a,int(s)) for a,s,*_ in (l.split() for l in open('work/zcap/todo.tsv'))]
ok=lambda a: a not in bad and res.get(a) in (None,'fail') and not os.path.exists(f'src/auto/fn_{a}.c') and os.path.exists(f'work/disasm/bundle_{a.lstrip("0")}.s')
pool=sorted([(a,s) for a,s in rows if lo<=s<=hi and pairs.get(a,0)>=mp and ok(a)],key=lambda x:x[1] if order=='small' else -x[1])
sel=pool[:30]; print('pool',len(pool),'taking',len(sel))
for i,w in enumerate(['G1','G2','G3']):
    open(f'work/zcap/agy{N}_{w}.list','w').write(''.join(f'{a}\t{z}\n' for a,z in sel[i::3]))
with open('work/zcap/strata_results.tsv','a') as f:
    for a,z in sel: f.write(f'{a}\t{z}\tagy\tassigned to {"codewhale" if eng=="cw" else "agy hydrogen"} (round {N})\n')
PY
for w in G1 G2 G3; do
  W=/PROCESOS/wt_$w; [ -s work/zcap/agy${N}_$w.list ] || continue
  git worktree remove --force $W 2>/dev/null; git worktree add -q --detach $W HEAD
  mkdir -p $W/work; for f in ram disasm maincpu.bin; do ln -sfn $ROOT/work/$f $W/work/$f; done
  cp work/zcap/pitfalls.txt work/zcap/m68kdis.py $W/work/
  cut -f1 work/zcap/agy${N}_$w.list > $W/work/batch.list
  sed "s|WTDIR|$W|" work/zcap/cw_prompt.txt > work/zcap/agy_prompt_$w.txt
  (cd $W && make -s -j8 build/test_fn >/dev/null 2>&1)
done
for w in G1 G2 G3; do
  [ -s work/zcap/agy${N}_$w.list ] || continue
  if [ "$ENGINE" = cw ]; then
    (cd /PROCESOS/wt_$w && codewhale exec --auto --sandbox workspace-write "$(cat $ROOT/work/zcap/agy_prompt_$w.txt)" > $ROOT/work/zcap/agy${N}_$w.log 2>&1) &
  else
  (cd /PROCESOS/wt_$w && agy --model ${AGY_MODEL:-gemini-4-hydrogen-medium} --mode auto --sandbox -p "$(cat $ROOT/work/zcap/agy_prompt_$w.txt)" > $ROOT/work/zcap/agy${N}_$w.log 2>&1) &
  fi
done; wait
grep -h 'quota\|RESOURCE_EXHAUSTED' work/zcap/agy${N}_G*.log | head -2
echo "$ENGINE round $N done"
