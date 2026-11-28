# elvactr plan (goal: at least 50% decompiled, Strata first; started 2026-11-28)

Metric: make coverage-elvactr (code bytes of elvactr/work/functions.tsv, 1600 functions, 172,914 B).
50% = 86,457 B. 70% of the bytes are in functions <= 400 B (Strata), 31% in 52 functions > 400 B
(codewhale / codex).

Scratchpad S=/tmp/claude-1000/-PROCESOS-Taitof3Decomp/337f8171-ed62-4db5-b4f1-08ab5208c52a/scratchpad
Push: temp GIT_ASKPASS script printing ~/.token.bak, deleted after. Commits end with
"Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>". Kill by PID only.

## Running (13:15)
- Capture batch: scripts/capture_game.sh over elvactr/work/capture_todo.txt (1164 executed entries),
  6 parallel, log elvactr/work/capture_log.tsv ("ADDR state pairs=N"; "-" = no pairs).
- Strata loop: scripts/strata_game_loop.sh elvactr 40, PID in $S/ea_strata.pid,
  log elvactr/work/strata_loop.log, results elvactr/work/strata_results.tsv.
  Accepted files land in elvactr/src/ea_<ADDR>.c + ea_auto.h + elvactr/tests/fn_auto.inc.

## Every check (hourly)
1. make -s test-elvactr must PASS; commit new elvactr/src/ea_*.c, ea_auto.h, elvactr/tests/fn_auto.inc,
   elvactr/work/strata_results.tsv; make coverage-elvactr; commit README.md + elvactr/docs; push.
2. Capture batch finished -> second pass on the "-" rows with CAP_STATES="P_play5000 P_play7000
   A_attract2400 A_attract4000 P_play2500" CAP_MAXF=3000. Still nothing -> injected captures
   (F3_INJECT, as dariusg recipes) later.
3. Large captured routines (> 400 B) -> codewhale / codex lanes in worktrees (wt_EA1, wt_EA2):
   give them the FN1 build command from scripts/strata_game.py build_run, one routine per file,
   accepted only bit-exact + strong (empty function fails).
   Running (14:30): codewhale cw1/cw2/cw3 (PIDs 143982-4) on the 15 captured routines > 400 B,
   drafts in elvactr/work/drafts/ea_<A>.c, logs drafts/cw{1,2,3}.log. Accept the BIT-EXACT ones with
   `scripts/check_ea.py accept <A> elvactr/work/drafts/ea_<A>.c` (refuses weak captures), then make test-elvactr.
   15:00: 5 accepted (0C6C18 refused: weak). cw4-6 (PIDs 303255/303530/303672) on 24 mid routines
   150-400 B (list elvactr/work/drafts/mid.txt). codewhale >> Strata in throughput: keep feeding lanes
   from captured, untried routines sorted by size.
   Second capture pass running -> elvactr/work/capture_log2.tsv (651 rows).
4. Strata loop idle -> relaunch with --retries 2 on its "fail" rows once.
5. Weak rows -> recapture with other states before giving up.

## 16:10
- Second capture pass done: 441/651 captured; 210 still without pairs (elvactr/work/capture_todo3.txt) -> injection.
- Lanes cw1-6 finished. Wave 3: cw7-12 (PIDs 784920 785157 785523 785716 785819 786153) on 60 routines
  (elvactr/work/drafts/wave3.txt, largest first; 409 captured untried routines = 64,678 B remain).
- Each check: accept loop over elvactr/work/drafts/ea_*.c with scripts/check_ea.py accept, then refill lanes.
