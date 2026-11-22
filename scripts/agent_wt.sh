#!/usr/bin/env bash
# Launch ONE CodeWhale subagent in its own in-repo worktree (.worktrees/<name>),
# populated with everything a routine-translation worker needs. The worktree has
# to live *under* the repo: the subagent tool refuses a cwd outside the parent
# workspace, and a plain `git worktree add` elsewhere leaves work/disasm,
# work/ram, roms, states and build behind (all gitignored).
#
# Usage: scripts/agent_wt.sh <name> [base-ref]     -> prepare, prints the cwd
#        scripts/agent_wt.sh --remove <name>
# Then start the worker with:
#   agent(action="start", type="implement", name="<name>",
#         cwd=".worktrees/<name>", write_roots=["."], prompt="<batch prompt>")
# Batch prompts: work/drafts/worker_prompt.md is copied into the worktree's
# work/ so the worker can read it without leaving its sandbox.
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
WTDIR="$ROOT/.worktrees"

if [ "${1:-}" = "--remove" ]; then
  n=$2
  d="$WTDIR/$n"
  git worktree remove --force "$d" 2>/dev/null || rm -rf "$d"
  git branch -D "wt/$n" >/dev/null 2>&1
  echo "removed $d"
  exit 0
fi

name=$1
base=${2:-HEAD}
d="$WTDIR/$name"
[ -e "$d" ] && { echo "exists: $d"; exit 1; }
mkdir -p "$WTDIR"
grep -qx '.worktrees/' .gitignore || printf '.worktrees/\n' >> .gitignore
git worktree add -q -b "wt/$name" "$d" "$base" || exit 1
# artifacts a worker needs but git does not carry
for p in work/maincpu.bin work/audiocpu_win.bin work/disasm work/ram roms states build \
         tools/pyghidra-mcp/.venv; do
  [ -e "$ROOT/$p" ] || continue
  if [ -f "$ROOT/$p" ]; then
    mkdir -p "$d/$(dirname "$p")"; cp "$ROOT/$p" "$d/$p"
  else
    mkdir -p "$d/$p"; rsync -a --exclude 'log/' "$ROOT/$p/" "$d/$p/"
  fi
done
cp scripts/wt_sweep.sh scripts/export_bundle.sh scripts/close_jump_tables.py "$d/scripts/" 2>/dev/null
cp work/drafts/worker_prompt.md "$d/work/" 2>/dev/null
chmod -R a+rX "$d"
echo "cwd=.worktrees/$name  bundles=$(ls "$d"/work/disasm/bundle_*.s 2>/dev/null | wc -l) captures=$(ls "$d"/work/ram/obj_*_pairs.bin 2>/dev/null | wc -l) disk=$(du -sh "$d" 2>/dev/null | cut -f1)"
