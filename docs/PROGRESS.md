# Progress — dariusg (Darius Gaiden, Taito F3)

Status legend: `TODO` / `MAPPED` (named, purpose known) / `DECOMPILED` (C in `src/`) / `VERIFIED` (C checked against an oracle).

## Phase gates

| Phase | Gate | Status | Evidence |
|---|---|---|---|
| 0 | mame-mcp + pyghidra-mcp wired in Hermes | DONE | `~/.hermes/config.yaml` |
| 1 | `work/maincpu.bin` reproducible | DONE | `scripts/reconstruct_rom.py`: MAME dump == offline rebuild (2 MiB, md5 27be2aa1…) |
| 1 | Vector table coherent | DONE | SSP=0x41FFFC, Reset=0x010000, IRQ2/3/5=0x1A34/0x1A54/0x1A6C, TRAP1..12 used, rest → 0x400 |
| 2 | Ghidra bootstrap with HW map | DONE | `scripts/ghidra/bootstrap.sh`: 563 fn, XREFs to CTRL_IO/SPRITE/PF/DPRAM; A5=0x408000 context; `fix_flow.py` repairs 22 code-over-data conflicts (e.g. Task0 0x1019C) |
| 3 | MMIO sweep A (I/O + audio) | DONE | `traces/mmio/sweepA_io_audio.log` (590 unique, 1200 frames attract) |
| 3 | MMIO sweep B (video) | DONE | `traces/mmio/sweepB_video.pcs.tsv` (158 PC/region rows, 150k unique accesses, 1200 frames attract) |
| 3 | Savestates `states/0x_*.sta` | DONE | `scripts/lua/session_ramdiff.lua` → 01_attract, 02_stage1_start, 03_active_combat (local, not versioned) |
| 3 | `docs/RAM_MAP.md` | DONE (initial) | players[2] 0x41B190, score 0x41B1D8, object pool, collision lists, terrain bitmap |

## Functions

| ROM | Name | Subsystem | Status | Notes |
|---|---|---|---|---|
| 0x010172 | Task0_Main | flow | MAPPED | boot → attract ↔ game ↔ test mode supervisor, see docs/GAME_FLOW.md |
| 0x0050DC | Eeprom_VerifyOrInit | io | MAPPED | 'TAITO' magic |
| 0x004D06 | Eeprom_UnpackSettings | io | MAPPED | free play, auto start, volume, ... |
| 0x0039D8 | Sound_PlayCmd | audio | VERIFIED (MAME, exact pre/post incl. DPRAM) | `src/audio/sound_cmd.c`; all 154 command ids forced via `F3_SWEEP` (types 0/2/3; types 1/4 unused by the table) + 100 natural calls |
| 0x010000 | Reset | boot | MAPPED | HW init, `lea $408000,A5`, → 0x402 POST |
| 0x000402 | (POST) | boot | TODO | LINE_RAM/RAM test loop, error screen |
| 0x001DB8 | Kernel_ColdBoot | kernel | MAPPED | spawns Task0_Main @0x10172 |
| 0x001A34 | IRQ2_VBlank | kernel | MAPPED | handshake with IRQ3, then Kernel_Preempt |
| 0x001A54 | IRQ3_VideoSync | kernel | MAPPED | |
| 0x001A6C | IRQ5_Timer | kernel | MAPPED | irq5_counter++ |
| 0x00222E | Kernel_Preempt | kernel | MAPPED | see docs/KERNEL.md |
| 0x002274 | Kernel_Schedule | kernel | MAPPED | 16 tasks, status bits 31/30/29 |
| 0x001E60.. | Sys_* (TRAP 1..12) | kernel | MAPPED | spawn/kill/exit/sleep/yield/recv/wait/block/send/signal/wake/flush |
| 0x001A0E | Kernel_FrameTick | kernel | MAPPED | calls 0x1C90, 0x1A78, 0xDFA, 0x161E, 0xBF6 |
| 0x001D8A | Rng_Next | kernel | VERIFIED (MAME) | `src/kernel/rng.c`; 600 frame pairs of rng_seed from MAME all reachable (avg 2385 idle-loop calls/frame) |
| 0x001A78 | IO_PollInputs | io | MAPPED | |
| 0x001B8E | IO_EdgeDetect | io | VERIFIED (truth table) | `src/io/input.c` |
| 0x001C90 | Video_UploadScroll | video | MAPPED | |
| 0x0C691A | Player_MoveAndBounds | player | VERIFIED (MAME, all branches) | `src/player/player_move.c`; 301/301 natural frames + forced-branch pairs (`scripts/lua/session_branches.lua`): shake 95/95, terrain shield-break 98/98, terrain recoil 100/100 |
| 0x0CBE12 | Obj_MoveByAngle | player | VERIFIED (via above) | sin table 0x27E40, cos = +0x80 |
| 0x0C6632 | Terrain_TestPoint | collision | VERIFIED (via above) | forced full bitmap only; per-bit addressing not yet exercised with a real stage map |
| 0x0C63F2..0x0C6630 | Task_Collision body (Coll_PlayerShotsVsEnemies, Coll_PlayersVsHazards, Coll_ResetLists, Terrain_SyncScroll) | collision | VERIFIED (MAME, exact pre/post) | `src/collision/collision.c`; 300/300 debugger-captured pairs bit-exact over all Work RAM; 35 shot hits, 1 hazard hit, 20 bomb hits exercised |
| 0x0D0E04 | Score_OnHit | player | MAPPED | writer of score verified by tap |
| 0x0047E8.. / 0x00092A.. | EEPROM 93C46 bit-bang | io | TODO | from sweep A |
| 0x003960..0x003CDE | Sound DPRAM interface (ResetDpram, PlayCmdForced, StopAll, SetVolume) | audio | DECOMPILED | `src/audio/sound_cmd.c`; PlayCmdForced 2/2, StopAll 2/2, StopAllPart2 1/1 captured calls bit-exact (rare in gameplay: low coverage); ResetDpram/SetVolume not yet captured. Ring = DPRAM 0x000-0x3FF, C00480 = write idx*2, C00482 = sound-CPU read idx |

## Task map (docs/GAME_FLOW.md)

| Slot | Entry | Name | Lifetime |
|---|---|---|---|
| 0 | 0x010172 | Task0_Main | forever |
| 4 | 0x010488 | Task_ObjectPoolRunner | attract + game (killed 0xFFFE at game over); ticks Obj_PlayersTick (ship movement) |
| 3 | 0x0C63E8 | Task_Collision | game: collisions + terrain (docs/PLAYER.md) |
| 2 | 0x144B60 | Task_StageBackground (inferred: per-stage table, line-RAM raster) | game |
| 5 | 0x0C6A64 | Task_PlayerControl (runs ship obj fns → Player_MoveAndBounds) | game |
| 6 | 0x13F2C6 | Task_StageTimeline (scroll-position-scripted enemy spawns) | game |
| 14 | 0x010502 | Task_ObjGroup11Runner (object pool group 11) | game |
| 15 | 0x0038AC | Task_Slot15Daemon (sound?) | survives game start (kill 0x7FFE) |
| 1 | 0x0B8404 | Task_Attract | attract only |
| 1 | 0x004146 | Task_GameMain | game only |
| 1 | 0x1A1A26 | Task_TestModeMenu | test mode |

Other spawn entries from `scripts/find_spawns.py` (spawned by sub-tasks, TODO): 010502, 0910CC, 0AC8E4, 0B935A, 0C63E8, 0C6A64, 13F2B6, 13F2C6, 13F436, 1439BC, 144B60, 1783A8.

## Object pool
| addr | name | status | notes |
|---|---|---|---|
| 0x01057C | Obj_Spawn | VERIFIED (MAME, exact pre/post) | `src/objects/obj_pool.c`; 300/300 natural + 2/2 forced pool-full (`F3_POKE32`) |
| 0x0105EC | Obj_Free | VERIFIED (MAME) | 300/300 |
| 0x0104B8 | Obj_RunGroups | VERIFIED (MAME, call order) | 60 frames, 2982 object ticks: same (fn, obj) sequence via `F3_LOG=104EE` |
| 0x01060E / 0x010638 / 0x010672 / 0x010532 | Obj_ResetGroups / ResetPools / ResetGroup11 / RunGroup11 | DECOMPILED | not yet captured |

## Stage
| addr | name | status | notes |
|---|---|---|---|
| 0x13F2D4..0x13F340 | StageTimeline_Step (Task_StageTimeline frame body) | VERIFIED (MAME, all branches) | `src/stage/stage_timeline.c`; 600/600 natural frames (8 waves), forced end marker 1/1, forced fx start 20/20 |
| 0x0015D0 / 0x0015EC | Fx_StartA / Fx_StartB | VERIFIED (A, via above) / DECOMPILED (B) | timed-effect tables 0x16890 / 0x168C0 |
| 0x13F342.. | stage-end sequence (boss warning spawns into group 11, waits, Fx_StartA(5)) | MAPPED | |

## Video
| addr | name | status | notes |
|---|---|---|---|
| 0x001282 | Obj_DrawSprite | VERIFIED (MAME, Work RAM + sprite RAM) | `src/video/sprite.c`; 400/400 natural + flip modes 1/2/3 forced (`F3_POKEA0B=1C=0n`) 150/150 each |

## Object routines
Drafts by agy (Gemini), verified here with per-object captures (`F3_COND="a1==<fn>"` at 0x104EE, Work RAM + sprite RAM).
| addr | name | status | notes |
|---|---|---|---|
| 0x102FE6 | Obj_EnemyBullet_Tick | VERIFIED (MAME) | 400/400 ticks; 2 death transitions seen; bomb-cancel path (0xCBB2C) and explosion spawn not yet exercised/decompiled |
| 0x0C9C6A | Obj_PlayerShot_Tick | VERIFIED (MAME) | 399/399 ticks, 5 frees; hit/explode path (0xCA970) not decompiled (1 tick skipped) |
| 0x0D9116 | Obj_D9116_Tick | VERIFIED (MAME) | 300/300 ticks (no state changes seen) |
| 0x0CABF4 | Obj_CABF4_Tick | VERIFIED (MAME) | 300/300 ticks (no state changes seen) |
| 0x0CF1E4 | ObjScript_CF1E4 (object script interpreter, 6-byte opcodes at obj+0x44) | VERIFIED (all 29 opcodes) | `src/objects/objscript_CF1E4.c`; 20/20 natural calls + every opcode forced through a synthetic script at 0x41E000 (always-zero RAM) via `F3_POKE32`/`F3_POKEA0B=44..47` (3/3 each; jump-back ops 17/24/25 with offset 0) |
| 0x0CF4B8 | ObjScript_CF4B8 (per-frame movement step, dispatch on mode obj+0x20, table 0xCF4C8; takes caller A1 = aim offset) | VERIFIED (all 29 modes) | 300/300 natural calls + every mode forced with `F3_POKEA0B=20=<mode>` (6/6 each). The 51-way scale table 0xD4F26 is modelled as a multiply and only verified for the speeds seen |
| 0x0EAD28 | Obj_EAD28_Tick (formation enemy using the script VMs; aims and fires at the nearer player) | VERIFIED (MAME) | 202/202 ticks incl. 4 deaths and 2 spawns. Draft by codewhale (agy auth expired, hermes out of context); fire decision 0xD09B8, angle helper 0xD0E52 table index, 0xD0C9E table base and the death tails 0xCEE48..0xCEEEE (A0 vs A1, live D0 low-byte spawn arg) fixed by Claude |
| 0x1027B4 / 0x102860 | Obj_1027B4_Tick / Obj_102860_Tick | VERIFIED (MAME) | 114/114 ticks each; drafts by hermes |
| 0x102928 / 0x10252E / 0x102A9A | Obj_*_Tick | VERIFIED (MAME) | 114/114 each (agy drafts, unchanged); no deaths in the captures |
| 0x0C9F2E | Obj_C9F2E_Tick | VERIFIED (MAME) | 142/142 incl. 8 alloc/free (agy draft; RNG now via Rng_NextMem) |
| 0x119990 | Obj_119990_Tick | VERIFIED (MAME) | 69/69 (agy) |
| 0x0CBF5E / 0x119CBA / 0x117D06 / 0x13E5B2 / 0x0CAB36 | Obj_*_Tick | VERIFIED (MAME) | 69/69, 69/69, 132/132, 139/139, 129/129 (hermes drafts; 0x117D70 path in 117D06 not decompiled) |
| 0x119714 | Obj_119714_Tick | VERIFIED (MAME) | 69/69 (hermes) |
| 0x0CBF8A | Obj_CBF8A_Tick (generic mover; per-object callback table at obj+0x50, index obj+0x5C) | VERIFIED (MAME) | 200/200 (agy); only callback table 0xD54D0 (0xD54DC/0xD550A/0xD5594) implemented, other tables return -1 |
| 0x0DCDF2 / 0x0DC8BE / 0x0F30F6 | Obj_*_Tick | VERIFIED (MAME) | 97/97, 133/133 (3 state changes, 3 alloc/free), 100/100 (agy drafts, unchanged) |
| 0x0E04CC | Obj_E04CC_Tick (turret that follows the scroll) | VERIFIED (MAME) | 58/58 (codewhale draft; Claude rewrote the 0xE0654 state table, which is 1-based, and the animation helpers 0xD0678/0xD0690) |
| 0x0DECF8 | Obj_DECF8_Tick (gun attached to an E04CC parent) | VERIFIED (MAME) | 56/56 (codewhale draft; Claude fixed 16-bit address truncation, 0xD0DC2 fall-through, 0xD0090, 0xCFA7C and the 0xDF01E table). Paths outside the capture are less trusted |
| 0x0EACDE / 0x0E0278 / 0x117BDE / 0x0DC3F6 | Obj_*_Tick | VERIFIED (MAME) | 49/49, 56/56, 55/55, 44/44 (hermes drafts) |
| 0x0F302E | Obj_F302E_Tick | VERIFIED (MAME) | 4/4 incl. a spawn (hermes draft; Obj_Spawn group/ngroups were swapped) |

Note on `Obj_Spawn` argument order: the word pushed last (just before the fn long) is the group, the one before it is ngroups. `tests/test_objects` accepts `F3_ONLY=<substring>` to run a subset.
| 0x0D5464 | Obj_D5464_Tick | VERIFIED (MAME) | 150/150 incl. 5 state changes (hermes draft; fixed to use the RAM RNG seed via `Rng_NextMem`) |
| 0x0CEE7C / 0x0CEE84 / 0x0CEE8C | Obj_Dying1/2/3_Tick | VERIFIED (MAME) | 8/8 each; frees the object on the 3rd frame |

Tools: `scripts/capture_obj.sh <addr>` (tries the savestates until enough ticks), `scripts/integrate_obj.sh <addr> <draft.c>` (header + Makefile + test entry). Shared `Rng_NextMem()` in `f3mem.h` (seed in RAM 0x406BAA).

Delegation workflow: `scripts/export_bundle.sh <addr>` writes `work/disasm/bundle_<addr>.s` (flow-following disassembly incl. callees and PC-relative jump tables, stopping at routines declared in `src/**/*.h`) → agy drafts the C → `F3_SPR=1 F3_COND="a1==<addr>" scripts/capture_fn.sh 104EE 104F0 300 work/ram/obj_<ADDR>` → `tests/test_objects.c`.

## Ghidra coverage
- `scripts/ghidra/obj_states.py` discovers object tick routines referenced only as immediates (`move.l #fn,(A0)`, `move.l #fn,-(SP)` before `jsr Obj_Spawn`) plus the ones observed in MAME (`traces/obj_ticks.txt`, 58 addresses from pool-runner logs). Clean bootstrap: 583 → 2527 functions; 58/58 observed tick routines are functions.

## Verification tooling
- `scripts/capture_fn.sh <entry> <exit> <n> <prefix>`: MAME debugger breakpoints (headless `-debug -debugger none`) dump Work RAM + registers exactly at a code region's entry/exit → golden pre/post pairs for any routine. Options: `F3_DPRAM` (sound DPRAM), `F3_SWEEP`/`F3_SWEEP_BASE` (force the word arg), `F3_POKE32` (force RAM at entry), `F3_LOG` (log registers at inner PCs, e.g. indirect calls), `F3_EXIT=ret` (post at the caller's return address; abandoned calls are dropped), `F3_MAXFRAMES` (watchdog: a stopped `-debug` MAME ignores SIGTERM, use `timeout -s KILL`), `F3_COND` (conditional entry, e.g. `a1==102fe6` at the pool call site 0x104EE to capture one object routine), `F3_SPR` (sprite RAM), `F3_POKEA0B` (byte pokes relative to A0). Tests share `tests/capture.h`.

## Known caveats
- Ghidra auto-analysis creates pointers over code reached only via `pea` (task entries); `fix_flow.py` must run after `apply_symbols.py` (bootstrap does this).
- Struct types (`symbols/dariusg_types.tsv`, `apply_types.py`) give field names in pseudo-C; `extraout_A0` noise remains because callees are not declared to preserve A0.
- Input timing: the object tick of frame t+1 sees io_ch polled at the start of that frame (test_player variant `RAM_t+1`).
- Auto-analysis made scalar constants into refs to SOUND_BANK/PIVOT_RAM/WORK_RAM_MIRROR (1 ref per function) — likely false XREFs, review before trusting.
- The repo's `ghidra_proj/taito_f3` is locked by Hermes' pyghidra-mcp while the gateway runs; bootstrap there once it is stopped.
