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

## Update 01:55
- codex usage limit again until 05:23. Round 12 lists ready (codex12_X*.list: 0B7D74 0910CC 0B7506 0B80A4, captures
  not passed in round 11). Add more captured leftovers to those lists before launching if any.
- 0x0D8F9A: codex C loops forever on its capture twice (copy in scratchpad fn_0D8F9A_codex.c): give it to codewhale.
- Do not merge codex changes to src/common/f3mem.[ch] (palette field) without updating every test initializer.

## Update 02:50
- Test-mode screens 1A1A26/19F504/1A0A52/1A1856/1A1944 committed (f458fbb). wt_CW2 now: 1A0942/1A00E6/1A043C/1A095E
  (log work/zcap/cw_tm2.log). Integrate: git diff in wt_CW2 > patch, git apply --3way, copy src/test/seg_*.c.
- NEXT for wt_CW2: DIFFICULTY 0x1A1282 (yield 0x1A12DC -> resume 0x1A12E2), captures seg_1A1282 (1), seg_1A12E2 (30),
  taken with F3_SERVICE=1 F3_SVC=90,360,99999 from TE_000750 (noffset 750).
- Test-mode captures: only 01_attract (noffset 0) and TE_/TM_ states replay; TG1_* states do not reproduce.
  Sound test reached with F3_SVC=90,150,99999 at frame 645 from 01_attract.

## Update 03:35
- 9d8212d: codewhale batch 8 (4 bit-exact + 0D8F9A 14/14 + 5 UNVERIFIED) and sound-test screens. 96.5% / 89.1%.
- capture_fn.lua fix: an injected call that resets the game re-ran the autoboot script and left NULs in regs.tsv
  (that was the 0D8F9A "infinite loop"). Now runs once + stops on reset. Scan found no other NUL-holed capture.
- wt_CW2 next: DIFFICULTY 0x1A1282 (see Update 02:50). wt_CW next: leftovers 13F436 0B7B52 (callee sets untranslated),
  boot 010000/000402/010172, 006020, 004D06, 005080, 144894.
- codewhale noted test-mode segs push jsr_pc+2 as the return address where the jsr is 4/6 bytes: check against captures.

## Update 04:30
- 2d10e62 (18 bit-exact 129xxx family) and 41d3eac (DIFFICULTY + menu dispatch). 97.6% / 89.4%.
- wt_CW: batch 11 = last 24 of the Strata queue (log cw_r11.log). wt_CW2: the 20 before those ($S/cw2_r1.list,
  log cw2_r1.log). Strata keeps the head of the queue; strata_fn skips files that already exist.
- cw_integrate.sh now applies edits to existing src/auto files too (wt_CW2 integrations still need the patch route
  for Makefile/seg.c/test_fn.c when it touches src/test).
- After the undecompiled ones: the 129 decompiled-but-unverified routines (weak/no capture) are what moves verified %.
