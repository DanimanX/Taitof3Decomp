#!/usr/bin/env bash
# Usage: scripts/capture_fn.sh <entry_hex> <exit_hex[,exit_hex...]> <samples> <out_prefix> [state]
# F3_DPRAM=1 captures sound DPRAM, F3_PAL=1 captures palette RAM, and F3_EEPROM=1 captures the EEPROM.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
F3_ENTRY="$1" F3_EXIT="$2" F3_SAMPLES="$3" F3_OUT="$4" \
  "$ROOT/scripts/mame_run.sh" "$ROOT/scripts/lua/capture_fn.lua" \
  -state "${5:-02_stage1_start}" -debug -debugger none
