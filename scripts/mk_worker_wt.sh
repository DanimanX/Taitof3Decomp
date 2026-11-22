#!/usr/bin/env bash
# Prepare a worker worktree that is a faithful copy of the main tree: every
# untracked-but-ignored artifact (work/disasm, work/ram, roms, states, build,
# tools/venvs) comes along, because routines are translated from work/disasm
# bundles and verified against work/ram captures.
# Usage: scripts/mk_worker_wt.sh <name>     -> ../Taitof3Decomp-wt/<name>
#        scripts/mk_worker_wt.sh --remove <name>
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEST_BASE="$(dirname "$ROOT")/Taitof3Decomp-wt"

if [ "${1:-}" = "--remove" ]; then
  d="$DEST_BASE/$2"
  git -C "$ROOT" worktree remove --force "$d" 2>/dev/null || rm -rf "$d"
  git -C "$ROOT" branch -D "wt/$2" >/dev/null 2>&1
  echo "removed $d"
  exit 0
fi

name=$1
d="$DEST_BASE/$name"
[ -e "$d" ] && { echo "exists: $d"; exit 1; }
# checkout at the current main HEAD (dirty main files are NOT carried over: a
# worktree cannot share them; main's own edits are committed by the parent)
git -C "$ROOT" worktree add -q -b "wt/$name" "$d" HEAD || exit 1
for p in work/maincpu.bin work/audiocpu_win.bin work/disasm work/ram roms states build tools/pyghidra-mcp/.venv; do
  [ -e "$ROOT/$p" ] || continue
  if [ -f "$ROOT/$p" ]; then
    rsync -a "$ROOT/$p" "$d/$(dirname "$p")/"
  else
    mkdir -p "$d/$p"
    rsync -a --exclude 'log/' "$ROOT/$p/" "$d/$p/"
  fi
done
cp "$ROOT"/scripts/wt_sweep.sh "$ROOT"/scripts/close_jump_tables.py "$ROOT"/scripts/export_bundle.sh "$d"/scripts/ 2>/dev/null
echo "worktree $d ready ($(du -sh "$d" 2>/dev/null | cut -f1), bundles $(ls "$d"/work/disasm/bundle_*.s 2>/dev/null | wc -l), captures $(ls "$d"/work/ram/obj_*_pairs.bin 2>/dev/null | wc -l))"
