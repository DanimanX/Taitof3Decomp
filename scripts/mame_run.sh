#!/usr/bin/env bash
# Run dariusg headless in MAME with a Lua autoboot script.
# Usage: scripts/mame_run.sh <script.lua> [extra mame args...]
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SCRIPT="$(readlink -f "$1")"; shift
cd "$ROOT"
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy exec mame dariusg -rompath "$ROOT/roms" \
  -video none -sound none -nothrottle -skip_gameinfo -noplugin data \
  -cfg_directory "$ROOT/.mame_mcp/cfg" -nvram_directory "$ROOT/.mame_mcp/nvram" \
  -state_directory "$ROOT/states" \
  -autoboot_script "$SCRIPT" -autoboot_delay 0 "$@"
