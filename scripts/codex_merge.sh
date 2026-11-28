#!/usr/bin/env bash
# codex_merge.sh N : merge the routines codex wrote in wt_X1..X3 (round N).
#   weak      : must pass its capture (test entry kept); merged as UNVERIFIED
#   nocapture : must build under the project flags and have NO test entry;
#               merged as UNVERIFIED
# Every merged file must start with the UNVERIFIED comment. Rows go to
# work/zcap/unverified.tsv. Then jt_scan (flagged ones are held back), full
# tests. Does not commit.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; cd "$ROOT"
N=${1:?round}
for w in ${LANES:-X1 X2 X3}; do
  W=/PROCESOS/wt_$w; [ -d $W ] || continue
  (cd $W && make -s build/test_fn >/dev/null 2>&1; timeout 1800 ./build/test_fn 2>&1 | grep -v "^f3mem" | head -c 20000000) > work/zcap/cmerge_$w.out 2>&1
done
python3 - "$N" <<'PY'
import os,re,shutil,subprocess,sys
N=sys.argv[1]
h=open('src/auto/fn_auto.h').read(); inc=open('tests/fn_auto.inc').read(); n=[]; held=[]
os.makedirs('work/zcap/partial',exist_ok=True)
for w in os.environ.get('LANES','X1 X2 X3').split():
    W=f'/PROCESOS/wt_{w}'
    if not os.path.exists(f'work/zcap/codex{N}_{w}.list'): continue
    kinds=dict(l.split() for l in open(f'work/zcap/codex{N}_{w}.list') if l.strip())
    out=open(f'work/zcap/cmerge_{w}.out').read()
    passed={m.group(1) for m in re.finditer(r'Fn 0x(\w+) auto\s+(\d+)/\s*(\d+) calls',out) if m.group(2)==m.group(3) and int(m.group(3))>0}
    wh=open(f'{W}/src/auto/fn_auto.h').read().splitlines(); wi=open(f'{W}/tests/fn_auto.inc').read().splitlines()
    for a,k in kinds.items():
        f=f'{W}/src/auto/fn_{a}.c'
        if not os.path.exists(f): held.append((a,'not written')); continue
        src=open(f).read()
        if k!='capture' and 'UNVERIFIED' not in '\n'.join(src.split('\n')[:6]): held.append((a,'no UNVERIFIED tag')); shutil.copy(f,'work/zcap/partial/'); continue
        c=subprocess.run(['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-iquote','src/auto','-c',f,'-o','/dev/null'],capture_output=True)
        if c.returncode: held.append((a,'does not build')); shutil.copy(f,'work/zcap/partial/'); continue
        line=[x for x in wi if f'fn_{a}"' in x]
        if k=='weak' and (not line or a not in passed): held.append((a,'weak: capture not passed')); shutil.copy(f,'work/zcap/partial/'); continue
        if k=='capture':
            if not line or a not in passed: held.append((a,'capture not passed')); shutil.copy(f,'work/zcap/partial/'); continue
        elif k=='nocapture' and line: held.append((a,'nocapture with a test entry')); shutil.copy(f,'work/zcap/partial/'); continue
        if 'return -1' in src: pass  # allowed, listed below
        pr=[x for x in wh if f'Fn_{a}(' in x]
        if not pr: held.append((a,'no prototype')); continue
        shutil.copy(f,'src/auto/')
        if pr[0]+'\n' not in h: h=h.replace('/* END generated */',pr[0]+'\n/* END generated */')
        if line and f'fn_{a}"' not in inc: inc+=line[0]+'\n'
        n.append((a,k))
open('src/auto/fn_auto.h','w').write(h); open('tests/fn_auto.inc','w').write(inc)
with open('work/zcap/unverified.tsv','a') as f:
    for a,k in n:
        if k!='capture': f.write(f'{a}\t{k}\tcodex round {N}\n')
with open('work/zcap/strata_results.tsv','a') as f:
    for a,why in held: f.write(f'{a}\t0\tfail\tcodex round {N}: {why}\n')
print('merged',len(n),' '.join(a for a,k in n)); print('held',' '.join(f'{a}:{w}' for a,w in held))
PY
echo "-- new headers copied / existing files changed by codex (review these):"
for w in ${LANES:-X1 X2 X3}; do W=/PROCESOS/wt_$w; [ -d $W ] || continue
  for h in $(git -C $W status --short src | sed -n 's/^?? \(src\/.*\.h\)$/\1/p'); do cp $W/$h $h; echo "copied $h"; done
  git -C $W status --short src | grep '^ M' | grep -v 'fn_auto.h' | sed "s|^ M |$w changed |"
done
echo "-- -1 returns:"; for f in $(git status --short src/auto | sed -n 's/^?? //p'); do k=$(grep -c 'return -1' $f); [ $k -gt 0 ] && echo "$f $k"; done
echo "-- jump tables:"; python3 scripts/jt_scan.py | grep -v '^10C548\|^110EC6\|^0D96B2\|^0D9A24'
make -s -j16 build/test_fn build/test_objects 2>&1 | grep -m3 error; ./build/test_fn | tail -1; echo "objects failing: $(./build/test_objects | grep -ci fail)"
