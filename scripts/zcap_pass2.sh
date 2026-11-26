#!/usr/bin/env bash
# zcap_pass2.sh ADDR : for a routine with < 10 captured pairs, try every other
# zone where the hit survey saw it (earliest frame first); keep a new capture
# only if it has more pairs than the current one. Stops at >= 10 pairs.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; cd "$ROOT"
A=$1
cur() { awk -F"\t" '$2=="post"{n++} END{print n+0}' "work/ram/fn_${A}_regs.tsv" 2>/dev/null || echo 0; }
best=$(cur)
for f in work/zcap/hitsz/Z*.tsv; do
  z=$(basename "$f" .tsv); z=${z#Z}
  fr=$(awk -v a="$A" '$1==a{print $2}' "$f"); [ -n "$fr" ] || continue
  echo "$fr $z"
done | sort -n | while read fr z; do
  [ "$best" -ge 10 ] && break
  t="work/ram/zc2_${A}"
  ZCAP_OUT=$t scripts/zcap_one.sh "$A" "$z" "$fr" >/dev/null
  n=$(awk -F"\t" '$2=="post"{n++} END{print n+0}' "${t}_regs.tsv" 2>/dev/null || echo 0)
  if [ "$n" -gt "$best" ]; then
    for s in pairs regs gfx spr; do [ -f "${t}_$s.bin" ] && mv "${t}_$s.bin" "work/ram/fn_${A}_$s.bin"; done
    mv "${t}_regs.tsv" "work/ram/fn_${A}_regs.tsv"; best=$n; echo "$A Z$z pairs=$n"
  fi
  rm -f "${t}"_*
done
echo "$A final=$(cur)"
