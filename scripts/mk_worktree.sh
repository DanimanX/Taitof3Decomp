#!/usr/bin/env bash
# Create a worker worktree for parallel drafting/fixing (agents, codewhale).
# Usage: scripts/mk_worktree.sh <name>  -> ../Taitof3Decomp-wt/<name>
# Git-ignored inputs (captures, ROM image, disassembly) are symlinked.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
WT="$ROOT/../Taitof3Decomp-wt/$1"
git -C "$ROOT" worktree add -q --detach "$WT" HEAD
mkdir -p "$WT/work"
for f in ram disasm maincpu.bin drafts; do ln -sfn "$ROOT/work/$f" "$WT/work/$f"; done
echo "$WT"
