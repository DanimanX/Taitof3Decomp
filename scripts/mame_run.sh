#!/usr/bin/env bash
# Run a Taito F3 game headless in MAME with a Lua autoboot script.
# Usage: [F3_GAME=elvactr] scripts/mame_run.sh <script.lua> [extra mame args...]
# F3_GAME defaults to dariusg.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SCRIPT="$(readlink -f "$1")"; shift
cd "$ROOT"
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy exec mame "${F3_GAME:-dariusg}" -rompath "$ROOT/roms" \
  -video none -sound none -nothrottle -skip_gameinfo -noplugin data \
  -cfg_directory "$ROOT/.mame_mcp/cfg" -nvram_directory "$ROOT/.mame_mcp/nvram" \
  -state_directory "$ROOT/states" \
  -autoboot_script "$SCRIPT" -autoboot_delay 0 "$@"
