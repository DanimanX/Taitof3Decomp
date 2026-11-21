#!/usr/bin/env bash
# Import work/maincpu.bin into a Ghidra project with the Taito F3 memory map,
# run auto-analysis, repair broken fall-throughs, apply symbols/traces and
# print the Phase-2 gate report.
# Usage: scripts/ghidra/bootstrap.sh [projdir] [projname]
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PROJDIR="${1:-$ROOT/ghidra_proj}"
PROJNAME="${2:-taito_f3}"
G="${GHIDRA_INSTALL_DIR:-/home/danimanx/opt/ghidra_12.1.4_PUBLIC}"
PY="${PYGHIDRA_PY:-$ROOT/tools/pyghidra-mcp/.venv/bin/python}"
export JAVA_HOME="${JAVA_HOME:-/usr/lib/jvm/java-21-openjdk}" GHIDRA_INSTALL_DIR="$G"
L="$G/Ghidra/Features/PyGhidra/support/pyghidra_launcher.py"
"$PY" "$L" "$G" -H "$PROJDIR" "$PROJNAME" -import "$ROOT/work/maincpu.bin" \
  -loader BinaryLoader -loader-baseAddr 0 -processor 68000:BE:32:MC68020 \
  -scriptPath "$ROOT/scripts/ghidra" -preScript setup_taito_f3.py \
  -overwrite
# Second pass without auto-analysis: fix_flow's repairs would otherwise be
# undone by the analysis that headless re-runs after scripts. fix_flow runs
# after apply_symbols/import_traces because those disassemble new entry points
# (e.g. Task0_Main 0x10172, only referenced via pea) that hit the conflicts.
"$PY" "$L" "$G" -H "$PROJDIR" "$PROJNAME" -process maincpu.bin -noanalysis \
  -scriptPath "$ROOT/scripts/ghidra" \
  -postScript apply_symbols.py -postScript import_traces.py -postScript fix_flow.py -postScript obj_states.py -postScript fix_flow.py \
  -postScript obj_states.py -postScript fix_flow.py -postScript obj_states.py -postScript fix_flow.py \
  -postScript apply_types.py \
  -postScript report_taito_f3.py
