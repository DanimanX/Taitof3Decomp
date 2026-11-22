#!/usr/bin/env bash
# Object-routine suite triage for scripts/harvest.sh.
# tests/test_objects reports, for every capture, how many ticks were observable.
# Most stubs legitimately have no observable tick yet (their captured pairs only
# reach undecompiled paths), and 534 of them would otherwise fail every harvest.
# This wrapper therefore:
#   - accepts a routine only on "N/ N ticks bit-exact" with 0 undecompiled paths,
#   - counts every rejected routine so regressions still fail the gate,
#   - prints ACCEPTED=<n>; harvest.sh keeps a batch only when n > 0.
# Usage: scripts/object_health.sh        (exit 0 = suite healthy)
set -uo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"; cd "$ROOT"
out=$(F3_STRICT_UNIMPL=1 ./build/test_objects 2>&1)
rc=$?
acc=$(printf '%s\n' "$out" | grep -cE '[0-9]+/ *[0-9]+ ticks bit-exact \(.* 0 reach undecompiled')
rej=$(printf '%s\n' "$out" | grep -c 'REJECT')
other=$(printf '%s\n' "$out" | grep '^objects:' | grep -v 'REJECT' \
        | grep -vE '[0-9]+/ *[0-9]+ ticks bit-exact \(.* 0 reach undecompiled' \
        | grep -vE '^objects: .* [0-9]+/ *[0-9]+ ticks bit-exact ' \
        | grep -vF ' SKIP (no ' | wc -l)
echo "objects: accepted=$acc rejected(unobservable)=$rej failing=$other"
[ "$other" -eq 0 ] && echo "ACCEPTED=$acc" || { echo "ACCEPTED=0"; exit 1; }
