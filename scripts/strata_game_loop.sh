#!/usr/bin/env bash
# Keep Strata drafting <game> routines while captures arrive: rounds of
# scripts/strata_game.py; stops after 3 rounds with nothing new to try.
# Usage: scripts/strata_game_loop.sh [game] [limit]
cd "$(dirname "$0")/.."
G=${1:-elvactr} L=${2:-40} idle=0
while [ $idle -lt 3 ]; do
  n0=$(wc -l < $G/work/strata_results.tsv 2>/dev/null || echo 0)
  python3 scripts/strata_game.py --game $G --limit $L
  n1=$(wc -l < $G/work/strata_results.tsv 2>/dev/null || echo 0)
  if [ "$n1" = "$n0" ]; then idle=$((idle+1)); sleep 600; else idle=0; fi
done
echo "strata_game_loop: idle, stopping"
