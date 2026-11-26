#!/usr/bin/env bash
# agy_round.sh N [MINSIZE] [MAXSIZE] : pick the next 30 untried captured routines
# (largest first within the size range), reset wt_G1..G3 at HEAD, and run three
# self-verifying agy (Gemini 4 Hydrogen) batches of 10. Logs: work/zcap/agyN_G*.log
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; cd "$ROOT"
N=$1 LO=${2:-1} HI=${3:-300}
python3 - "$N" "$LO" "$HI" <<'PY'
import os,sys
N,lo,hi=sys.argv[1],int(sys.argv[2]),int(sys.argv[3])
res={}
for l in open('work/zcap/strata_results.tsv'):
    p=l.rstrip('\n').split('\t'); res[p[0]]=p[2]
bad={'13A0EE','13C852','19C648','12AC6C','18CAB4','193096','1346B6','1100C6','105356','10EDA0','13C43E'}
pairs={l.split()[0]:int(l.split()[1]) for l in open('work/zcap/pairs_now.txt')}
rows=[(a,int(s)) for a,s,*_ in (l.split() for l in open('work/zcap/todo.tsv'))]
ok=lambda a: a not in bad and res.get(a) in (None,'fail') and not os.path.exists(f'src/auto/fn_{a}.c') and os.path.exists(f'work/disasm/bundle_{a.lstrip("0")}.s')
pool=sorted([(a,s) for a,s in rows if lo<=s<=hi and pairs.get(a,0)>=3 and ok(a)],key=lambda x:-x[1])
sel=pool[:30]; print('pool',len(pool),'taking',len(sel))
for i,w in enumerate(['G1','G2','G3']):
    open(f'work/zcap/agy{N}_{w}.list','w').write(''.join(f'{a}\t{z}\n' for a,z in sel[i::3]))
with open('work/zcap/strata_results.tsv','a') as f:
    for a,z in sel: f.write(f'{a}\t{z}\tagy\tassigned to agy hydrogen (round {N})\n')
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
  (cd /PROCESOS/wt_$w && agy --model gemini-4-hydrogen-high --mode auto --sandbox -p "$(cat $ROOT/work/zcap/agy_prompt_$w.txt)" > $ROOT/work/zcap/agy${N}_$w.log 2>&1) &
done; wait
grep -h 'quota\|RESOURCE_EXHAUSTED' work/zcap/agy${N}_G*.log | head -2
echo "agy round $N done"
