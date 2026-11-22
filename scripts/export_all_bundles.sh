#!/usr/bin/env bash
# Export the flow-following disassembly bundle for every function in
# work/disasm_missing.txt that has no bundle yet (depth 3, default stop list =
# public routines declared in src/**/*.h). Resumable: existing bundles are
# skipped, so a second run after a crash continues where it stopped.
# Ghidra is opened read-only (-readOnly) on ${GHIDRA_PROJDIR:-ghidra_proj}, so
# this is safe to run while nothing else writes the project.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
LIST=${1:-work/disasm_missing.txt}
mkdir -p work/disasm work/disasm/log
done_n=0; fail_n=0; skip_n=0
while read -r a; do
  [ -z "$a" ] && continue
  if [ -e "work/disasm/bundle_$a.s" ]; then skip_n=$((skip_n+1)); continue; fi
  out=$(timeout 600 scripts/export_bundle.sh "$a" 3 2>&1 | tail -3)
  if [ -s "work/disasm/bundle_$a.s" ]; then
    done_n=$((done_n+1))
  else
    fail_n=$((fail_n+1)); echo "$a FAIL: $out" >> work/disasm/log/failures.txt
  fi
  [ $(((done_n + fail_n) % 25)) -eq 0 ] && echo "progress: done=$done_n fail=$fail_n skip=$skip_n ($a)"
done < "$LIST"
echo "EXPORT done=$done_n fail=$fail_n skip=$skip_n"
