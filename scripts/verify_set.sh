#!/usr/bin/env bash
# Verify a coherent set of drafts from one worktree (a file may need its .h and
# a sibling that exports a shared helper). Usage: verify_set.sh <wt> <ADDR1,..>
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; WT=$1; SET=$2
cd "$ROOT"
IFS=',' read -ra ADDRS <<< "$SET"
for a in "${ADDRS[@]}"; do
  for f in "src/enemy/obj_$a.c" "src/enemy/obj_$a.h"; do
    [ -e "$WT/$f" ] || continue
    if [ -e "$f" ]; then git checkout -q -- "$f" 2>/dev/null; fi
    cp "$WT/$f" "$f"
  done
done
if ! make build/test_objects > /tmp/setbuild.log 2>&1; then
  echo "BUILD FAILED"; grep -E "error:|Error " /tmp/setbuild.log | head -8; exit 2
fi
rc=0
for a in "${ADDRS[@]}"; do
  # accept on the strict line, or on the plain line when every tick was observable
  # plain run only: F3_STRICT_UNIMPL adds a second, REJECT-tagged line for
  # routines that have an unobservable pair even when others are exact
  r=$(F3_ONLY=$a timeout 180 ./build/test_objects 2>&1 | grep -E "^objects: .*0x0*$a\b" | tail -1)
  n=$(echo "$r" | grep -oE '[0-9]+/ *[0-9]+' | head -1 | tr -d ' ')
  un=$(echo "$r" | grep -oE '[0-9]+ reach undecompiled' | cut -d' ' -f1)
  rej=$(echo "$r" | grep -c REJECT)
  ok=no
  [ -n "$n" ] && [ "${n%%/*}" = "${n##*/}" ] && [ "${un:-1}" = "0" ] && [ "$rej" = "0" ] && [ "${n%%/*}" != "0" ] && ok=yes
  echo "$a ${n:-NONE} un=${un:-?} accept=$ok"
  [ "$ok" = yes ] || rc=1
done
if [ "$rc" != 0 ]; then
  for a in "${ADDRS[@]}"; do
    for f in "src/enemy/obj_$a.c" "src/enemy/obj_$a.h"; do
      [ -e "$WT/$f" ] || continue
      git checkout -q -- "$f" 2>/dev/null || rm -f "$f"
    done
  done
  echo "(set reverted)"
fi
