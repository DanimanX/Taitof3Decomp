#!/usr/bin/env bash
# Import <game>/work/maincpu.bin (a second Taito F3 game, e.g. elvactr) into
# its own Ghidra project with the F3 memory map, auto-analyse, repair broken
# fall-throughs and export <game>/work/functions.tsv.
# The Darius-only post scripts (symbols, MMIO traces, object states, types)
# are skipped until the game has its own data for them.
# Usage: [SEED="<game>/work/trace_r1 ..."] scripts/ghidra/bootstrap_game.sh <game>
# SEED: trace prefixes from scripts/trace_pcs.py (seed_trace.py disassembles
# every executed PC and makes a function at every call target).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
GAME="$1"
PROJDIR="$ROOT/$GAME/ghidra_proj"
G="${GHIDRA_INSTALL_DIR:-/home/danimanx/opt/ghidra_12.1.4_PUBLIC}"
PY="${PYGHIDRA_PY:-$ROOT/tools/pyghidra-mcp/.venv/bin/python}"
export JAVA_HOME="${JAVA_HOME:-/usr/lib/jvm/java-21-openjdk}" GHIDRA_INSTALL_DIR="$G"
L="$G/Ghidra/Features/PyGhidra/support/pyghidra_launcher.py"
mkdir -p "$PROJDIR"
"$PY" "$L" "$G" -H "$PROJDIR" "$GAME" -import "$ROOT/$GAME/work/maincpu.bin" \
  -loader BinaryLoader -loader-baseAddr 0 -processor 68000:BE:32:MC68020 \
  -scriptPath "$ROOT/scripts/ghidra" -preScript setup_taito_f3.py \
  -overwrite
"$PY" "$L" "$G" -H "$PROJDIR" "$GAME" -process maincpu.bin -noanalysis \
  -scriptPath "$ROOT/scripts/ghidra" \
  ${SEED:+-postScript seed_trace.py $SEED} \
  -postScript fix_flow.py -postScript report_taito_f3.py \
  -postScript export_functions.py "$ROOT/$GAME/work/functions.tsv"
