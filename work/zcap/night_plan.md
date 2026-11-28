# Night plan 2026-11-27/28 (goal: 100%, use Strata)

Scratchpad S=/tmp/claude-1000/-PROCESOS-Taitof3Decomp/337f8171-ed62-4db5-b4f1-08ab5208c52a/scratchpad
Push: temp GIT_ASKPASS script printing ~/.token.bak, deleted after. Commits end with
"Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>". Kill by PID only.

## Running
- Strata loop: scripts/strata_fn.py on $S/clean2.txt (138 strong, non-DPRAM routines <=200 B),
  log work/zcap/strata_fn_r10.log, PID in $S/strata_r10.pid. Started 21:16.
  Measure: accepted / processed. Accepted files land directly in src/auto + fn_auto.h/inc.
- codewhale in /PROCESOS/wt_CW (branch cw_work): 12 routines (183A2C 11A994 1785B2 178654 144378
  131088 0B760C 1781F6 11E11C 124C10 178700 0E2EAC), log work/zcap/cw_r1.log.

## At 00:20 (codex usage limit resets 00:12 -03)
- Lists ready: work/zcap/codex10_X{1,2,3}.list (66 captured undecompiled routines).
- Add "assigned to codex ... (round 10)" rows to work/zcap/strata_results.tsv, then
  `LISTS_READY=1 scripts/codex_round.sh 10 </dev/null > work/zcap/codex10.out 2>&1` (background).
- When done: `scripts/codex_merge.sh 10` (has timeout/capped output now), check_capture.sh on each
  merged routine (null passes all -> add to work/zcap/unverified.tsv as weak), commit, push.

## Every check (hourly)
1. Strata: if new src/auto/fn_*.c accepted by strata_fn (header "drafted by Strata"): make -s
   build/test_fn && full test_fn PASS -> commit those files + fn_auto.h + fn_auto.inc, push.
2. codewhale wt_CW finished: copy PASS routines' fn_*.c, prototypes, inc lines to main, test, commit.
   Then launch next codewhale batch from the leftovers of round 10 / strata fails.
3. Run scripts/gcov_build.sh then scripts/coverage.py; commit coverage files.
4. Never run test_fn without timeout + head -c on untrusted drafts.
5. If Strata loop finished: report rate; relaunch on its 'fail' routines with --retries 2 once.

## Update 01:10
- codex round 11 (18 captured routines) running; merge with scripts/codex_merge.sh 11.
- codewhale wt_CW batch 7: 16 no-capture routines (UNVERIFIED by design). Integrate: scripts/cw_integrate.sh /PROCESOS/wt_CW
- codewhale wt_CW2: test-mode segment groups 1A1A26, 19F504, 1A0A52, 1A1856, 1A1944 (seg_* captures). Integrate with
  scripts/cw_integrate.sh /PROCESOS/wt_CW2 PLUS copy src/test/seg_*.c, the Makefile SRC line, src/kernel/seg.c routing and
  tests/test_fn.c Seg lines by hand (cw_integrate only handles src/auto).
- Full test_fn takes ~5 s; if it runs for minutes a draft is looping: find it with F3_ONLY per entry + timeout.
- Still to plan: unreached screens 1A00E6, 1A043C, 1A1282, 1A095E; boot code 010000/000402/010172/01010A; 006020 (printf case).
