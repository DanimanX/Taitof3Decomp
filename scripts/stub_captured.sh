#!/usr/bin/env bash
# For each "<FN> <state> pairs=N" line (N>0) of a capture_batch log: rename the
# capture files to the stripped-zero convention, export the disasm bundle and
# integrate a stub (so workers can fill it). Skips routines already in src/.
# Usage: scripts/stub_captured.sh <capture.log> [skip_addr ...]
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; cd "$ROOT"
log=$1; shift; skip=" $* "
tmp=$(mktemp)
while read -r fn st pr; do
  [ "$pr" = "pairs=0" ] && continue
  a=$(printf %X $((16#$fn)))
  case "$skip" in *" $a "*) continue;; esac
  [ -e "src/enemy/obj_$a.c" ] && continue
  for x in pairs.bin regs.tsv spr.bin gfx.bin dpram.bin; do
    [ -e "work/ram/obj_${fn}_$x" ] && [ "$fn" != "$a" ] && mv "work/ram/obj_${fn}_$x" "work/ram/obj_${a}_$x"
  done
  [ -e "work/disasm/bundle_$a.s" ] || scripts/export_bundle.sh "$a" >/dev/null
  printf '#include "obj_%s.h"\n/* ROM 0x%06X: not decompiled yet (stub) */\nint Obj_%s_Tick(const f3mem_t *m, uint32_t obj) { (void)m; (void)obj; return -1; }\n' "$a" $((16#$a)) "$a" > "$tmp"
  scripts/integrate_obj.sh "$a" "$tmp" && echo "STUB $a ($pr)"
done < "$log"
rm -f "$tmp"
