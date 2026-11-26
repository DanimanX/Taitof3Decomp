#!/usr/bin/env bash
# agy_merge.sh [SKIP...] : test + audit wt_G1..G3, merge every new src/auto routine
# that passes into main except those listed (SKIP) and weak ones (an empty body
# changes no RAM differently); then jt_scan, full tests. Does not commit.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; cd "$ROOT"
SKIP=" $* "
for w in G1 G2 G3; do
  W=/PROCESOS/wt_$w; [ -d $W ] || continue
  (cd $W && make -s build/test_fn >/dev/null 2>&1; ./build/test_fn) > work/zcap/merge_$w.out
  scripts/audit_auto.sh $W > work/zcap/audit_$w.out
done
python3 - "$SKIP" <<'PY'
import re,shutil,sys
skip=set(sys.argv[1].split())
h=open('src/auto/fn_auto.h').read(); inc=open('tests/fn_auto.inc').read(); n=[]; held=[]
for w in ['G1','G2','G3']:
    W=f'/PROCESOS/wt_{w}'
    try: out=open(f'work/zcap/merge_{w}.out').read(); aud=open(f'work/zcap/audit_{w}.out').read()
    except FileNotFoundError: continue
    passed={m.group(1) for m in re.finditer(r'Fn 0x(\w+) auto\s+(\d+)/\s*(\d+) calls',out) if m.group(2)==m.group(3) and int(m.group(3))>0}
    wh=open(f'{W}/src/auto/fn_auto.h').read().splitlines(); wi=open(f'{W}/tests/fn_auto.inc').read().splitlines()
    for l in aud.splitlines()[1:]:
        p=l.split(); a=p[0]; nul=int(p[4].split('=')[1])
        if a in skip: held.append(a+':skip'); continue
        if a not in passed: held.append(a+':fail'); shutil.copy(f'{W}/src/auto/fn_{a}.c','work/zcap/partial/'); continue
        if nul==0: held.append(a+':weak'); shutil.copy(f'{W}/src/auto/fn_{a}.c','work/zcap/weak/'); continue
        shutil.copy(f'{W}/src/auto/fn_{a}.c','src/auto/')
        pr=[x for x in wh if f'Fn_{a}(' in x][0]+'\n'
        if pr not in h: h=h.replace('/* END generated */',pr+'/* END generated */')
        if f'fn_{a}"' not in inc: inc+=[x for x in wi if f'fn_{a}"' in x][0]+'\n'
        n.append(a)
open('src/auto/fn_auto.h','w').write(h); open('tests/fn_auto.inc','w').write(inc)
with open('work/zcap/strata_results.tsv','a') as f:
    for x in held:
        a,why=x.split(':'); f.write(f'{a}\t0\t{"weak" if why=="weak" else "fail"}\tagy: {why}\n')
print('merged',len(n),' '.join(n)); print('held',' '.join(held))
PY
echo "-- -1 returns / raw PlayCmd in merged:"; for w in G1 G2 G3; do awk '$3!="ret-1=0"||$4!="rawPlayCmd=0"' work/zcap/audit_$w.out 2>/dev/null | grep -v '^outside'; done
echo "-- jump tables:"; python3 scripts/jt_scan.py | grep -v '^10C548\|^110EC6'
make -s -j16 build/test_fn build/test_objects 2>&1 | grep -m3 error; ./build/test_fn | tail -1; echo "objects failing: $(./build/test_objects | grep -ci fail)"; make -s coverage | tail -1
