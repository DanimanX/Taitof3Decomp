#!/usr/bin/env bash
# Usage: scripts/export_bundle.sh <root_hex> [depth] [stop_hex,...]
# -> work/disasm/bundle_<root>.s (read-only on ghidra_proj/taito_f3)
set -euo pipefail
# GHIDRA_PROJDIR overrides the project dir (e.g. a copy, while hermes'
# pyghidra-mcp holds the lock on ghidra_proj)
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
G=${GHIDRA_HOME:-/home/danimanx/opt/ghidra_12.1.4_PUBLIC}
OUT="$ROOT/work/disasm/bundle_$1.s"
# stop list default: public routines declared in src/**/*.h (static helpers
# inside .c files are not reusable, so they are re-exported)
STOPS=${3:-$( grep -rhoE "ROM 0x[0-9A-Fa-f]{4,6}" "$ROOT/src" --include=*.h | grep -oE "0x[0-9A-Fa-f]{4,6}" | sed 's/0x//' | sort -u | paste -sd,)}
JAVA_HOME=${JAVA_HOME:-/usr/lib/jvm/java-21-openjdk} "$ROOT/tools/pyghidra-mcp/.venv/bin/python" \
  "$G/Ghidra/Features/PyGhidra/support/pyghidra_launcher.py" "$G" -H "${GHIDRA_PROJDIR:-$ROOT/ghidra_proj}" taito_f3 \
  -process maincpu.bin -noanalysis -readOnly -scriptPath "$ROOT/scripts/ghidra" \
  -postScript export_bundle.py "$OUT" "$1" "${2:-3}" "$STOPS" 2>&1 | grep -E "export_bundle|ERROR|Lock" || true
