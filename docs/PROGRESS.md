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
| 0x0C6866 / 0x0C9978 / 0x0C9D7E / 0x0DC542 / 0x0E02C4 | Obj_*_Tick | VERIFIED (MAME) | 69/69, 21/21 (21 state changes), 9/9 (9 state changes), 27/27, 56/56 (1 alloc/free) (agy drafts, unchanged) |
| 0x0CBF6C / 0x0CC0F4 / 0x0D540A / 0x0D9092 / 0x0DEC9E / 0x0D901A / 0x0DC456 | Obj_*_Tick (init/transition states) | VERIFIED (MAME) | 1/1, 9/9, 6/6, 1/1, 1/1, 5/5, 3/3 (agy drafts, unchanged). Short-lived states: few samples |
| 0x0E0266 / 0x0E029E / 0x102F52 / 0x117B44 / 0x0F30B4 / 0x117C28 / 0x13E4EE / 0x0DC586 | Obj_*_Tick (init/transition states) | VERIFIED (MAME) | 1/1, 1/1, 8/8, 1/1, 4/4, 1/1, 9/9, 2/2 (codewhale drafts, 0/8 as delivered; Claude fixed `lea d16(PC)` bases taken as insn+4 instead of +2, the 0xD2D44 Y table 0xD2ECC and X/Y swap, `divu.w` remainder/quotient, `move.l`+`andi.w` low word, missing (A1)+ in 0xCEC4C, invented writes, 13E4EE rewritten) |
| 0x0DC328 / 0x0DC0F8 | Obj_DC328_Tick (death explosion of the E04CC family, SFX 0x33) / Obj_DC0F8_Tick (its animated shot) | VERIFIED (MAME, forced) | 1/1, 32/32 with the parent's HP forced: `F3_COND="a1==e04cc \|\| a1==<fn>" F3_POKEFN=e04cc F3_POKEA0B=24=FF` on `03_active_combat` (codewhale drafts; DC328 PC-relative bases and sound call fixed) |
| 0x0DF7EA / 0x0DF844 | Obj_DF7EA_Tick (flare spawned by E02C4 when its parent reaches phase 5) / Obj_DF844_Tick (its constant-velocity shot) | VERIFIED (MAME, forced) | 40/40, 100/100 forcing the spawn on E02C4: `F3_COND="a1==e02c4 \|\| a1==<fn>" F3_POKEFN=e02c4 F3_POKEA0B=6=01,23=01 F3_POKEA0P=8:6=05` on `03_active_combat` (codewhale drafts, unchanged) |

| 0x0D7306 / 0x0E96C8 / 0x0BBE06 / 0x0BBFE8 / 0x0BC152 / 0x0BC638 / 0x0BCD90 / 0x0BCE38 / 0x0BD340 / 0x0CB0DA / 0x0CB406 / 0x0CBC88 / 0x0CBC90 / 0x0EACEA / 0x0ECAC8 / 0x0BC70A / 0x0ECA04 / 0x0F34F6 | Obj_*_Tick (survey batch) | VERIFIED (MAME) | 0x0D7306 63/63 (claude2), 0x0E96C8 100/100 (claude2), 0x0BBE06 100/100 (cw3), 0x0BBFE8 1/1 (cw3), 0x0BC152 1/1 (cw3), 0x0BC638 100/100 (cw3), 0x0BCD90 7/7 (cw3), 0x0BCE38 10/10 (cw3), 0x0BD340 80/80 (cw3), 0x0CB0DA 1/1 (cw3), 0x0CB406 1/1 (cw3), 0x0CBC88 7/7 (cw3), 0x0CBC90 7/7 (cw3), 0x0EACEA 8/8 (cw3), 0x0ECAC8 22/22 (cw3), 0x0BC70A 100/100 (cw4), 0x0ECA04 100/100 (cw4), 0x0F34F6 100/100 (cw4) |
| 0x10967C / 0x0BDE92 / 0x0E1328 / 0x0E1886 / 0x0ECB3A / 0x1049C4 / 0x0DB074 / 0x0DC6D0 / 0x0E9DE0 / 0x0EA45E / 0x0EA848 / 0x0DA138 / 0x0CAEBA / 0x0C9B3E / 0x0CAF1A | Obj_*_Tick (survey batch) | VERIFIED (MAME) | 0x10967C 100/100 (claude1), 0x0BDE92 32/32 (claude1), 0x0E1328 100/100 (claude1), 0x0E1886 100/100 (claude1), 0x0ECB3A 100/100 (claude1), 0x1049C4 100/100 (claude2), 0x0DB074 1/1 (claude2), 0x0DC6D0 8/8 (claude2), 0x0E9DE0 48/48 (claude3), 0x0EA45E 100/100 (claude3), 0x0EA848 100/100 (claude3), 0x0DA138 100/100 (cw1), 0x0CAEBA 1/1 (cw3), 0x0C9B3E 100/100 (cw4), 0x0CAF1A 6/6 (cw4) |
Note: 0x0BDE92 and 0x0BE262 (palette/scroll objects, attract-mode text typer) also write line RAM / playfield / text RAM (0x61xxxx-0x62xxxx), which `f3mem` does not model; those writes are left out and only their Work RAM effects are verified.

| 0x11C1DE / 0x12F27A / 0x0BBD4A / 0x0BBD6E / 0x0BC008 / 0x0BC03C / 0x0BC064 / 0x0BC3EC / 0x0BC418 / 0x0BD354 / 0x0BD544 / 0x0CABAC / 0x0CB412 / 0x0CBC98 / 0x12EF24 / 0x12F26A / 0x0BB746 / 0x0BB83E / 0x0BBBE0 / 0x0BBCDA / 0x0BBE9A / 0x0BC07E / 0x0BC3DE / 0x0BCA00 / 0x0BCE8E / 0x0BD256 / 0x0CA28C / 0x0CAF3E / 0x0F35B4 / 0x102798 / 0x12E19A / 0x0BBAE4 / 0x12EE24 / 0x11A87E / 0x12E21A / 0x0BBBF4 / 0x0BBD00 / 0x0BC08C / 0x0BC16C / 0x0BC40E / 0x0BC540 / 0x0BCE52 / 0x0BCFF4 / 0x11C39A / 0x12E1FA / 0x12EAD4 / 0x12EB0E / 0x0BB952 / 0x0BBD94 / 0x0BC24A / 0x0BC4A8 / 0x0BCD82 / 0x0BCDC0 / 0x0BCF08 / 0x0BD0EC / 0x0BD298 / 0x0BDC5C / 0x0BDDB8 / 0x0C685A / 0x0CB2C6 / 0x0D7570 | Obj_*_Tick (survey batch) | VERIFIED (MAME) | 0x11C1DE 1/1 (claude1), 0x12F27A 10/10 (claude1), 0x0BBD4A 100/100 (claude1), 0x0BBD6E 1/1 (claude1), 0x0BC008 100/100 (claude1), 0x0BC03C 1/1 (claude1), 0x0BC064 100/100 (claude1), 0x0BC3EC 1/1 (claude1), 0x0BC418 1/1 (claude1), 0x0BD354 64/64 (claude1), 0x0BD544 1/1 (claude1), 0x0CABAC 8/8 (claude1), 0x0CB412 21/21 (claude1), 0x0CBC98 7/7 (claude1), 0x12EF24 1/1 (claude2), 0x12F26A 1/1 (claude2), 0x0BB746 100/100 (claude2), 0x0BB83E 1/1 (claude2), 0x0BBBE0 1/1 (claude2), 0x0BBCDA 1/1 (claude2), 0x0BBE9A 1/1 (claude2), 0x0BC07E 1/1 (claude2), 0x0BC3DE 64/64 (claude2), 0x0BCA00 1/1 (claude2), 0x0BCE8E 2/2 (claude2), 0x0BD256 64/64 (claude2), 0x0CA28C 100/100 (claude2), 0x0CAF3E 1/1 (claude2), 0x0F35B4 100/100 (claude3), 0x102798 1/1 (cw1), 0x12E19A 1/1 (cw1), 0x0BBAE4 64/64 (cw1), 0x12EE24 100/100 (cw2), 0x11A87E 2/2 (cw3), 0x12E21A 100/100 (cw3), 0x0BBBF4 64/64 (cw3), 0x0BBD00 100/100 (cw3), 0x0BC08C 100/100 (cw3), 0x0BC16C 85/85 (cw3), 0x0BC40E 1/1 (cw3), 0x0BC540 1/1 (cw3), 0x0BCE52 100/100 (cw3), 0x0BCFF4 100/100 (cw3), 0x11C39A 1/1 (cw4), 0x12E1FA 2/2 (cw4), 0x12EAD4 100/100 (cw4), 0x12EB0E 8/8 (cw4), 0x0BB952 100/100 (cw4), 0x0BBD94 100/100 (cw4), 0x0BC24A 1/1 (cw4), 0x0BC4A8 100/100 (cw4), 0x0BCD82 6/6 (cw4), 0x0BCDC0 100/100 (cw4), 0x0BCF08 3/3 (cw4), 0x0BD0EC 100/100 (cw4), 0x0BD298 64/64 (cw4), 0x0BDC5C 1/1 (cw4), 0x0BDDB8 1/1 (cw4), 0x0C685A 2/2 (cw4), 0x0CB2C6 100/100 (cw4), 0x0D7570 3/3 (cw4) |
| 0x12EB90 / 0x12E1BE / 0x12EDA8 / 0x0BB86A / 0x0CB054 | Obj_*_Tick (survey batch) | VERIFIED (MAME) | 0x12EB90 100/100 (claude3), 0x12E1BE 2/2 (cw3), 0x12EDA8 1/1 (cw3), 0x0BB86A 1/1 (cw3), 0x0CB054 60/60 (cw3) |
| 0x105268 / 0x1095B0 / 0x1116E2 / 0x0BB5C2 / 0x0BB692 / 0x0BB860 / 0x0BBE50 / 0x0BC3FA / 0x0BE262 / 0x0CAF5E / 0x0CB2EC / 0x0CBC78 / 0x0DBDE8 / 0x102844 / 0x11C1FC / 0x13EFA0 / 0x0BC1E8 / 0x0BCC96 / 0x0BD308 / 0x0BD7E4 / 0x0BDDE2 / 0x0BDE42 / 0x0CB97E / 0x0E1B36 / 0x0ECFAE / 0x0F3582 / 0x10524A / 0x11A8A8 / 0x12EF40 / 0x0BC404 / 0x0BC680 / 0x0BD2C2 / 0x0EA836 / 0x0ECF0C / 0x0BBB18 / 0x0BC576 / 0x0EA28C / 0x0ECF3E | Obj_*_Tick (survey batch) | VERIFIED (MAME) | 0x105268 100/100 (claude1), 0x1095B0 100/100 (claude1), 0x1116E2 100/100 (claude1), 0x0BB5C2 100/100 (claude1), 0x0BB692 100/100 (claude1), 0x0BB860 1/1 (claude1), 0x0BBE50 100/100 (claude1), 0x0BC3FA 1/1 (claude1), 0x0BE262 100/100 (claude1), 0x0CAF5E 14/14 (claude1), 0x0CB2EC 100/100 (claude1), 0x0CBC78 100/100 (claude1), 0x0DBDE8 100/100 (claude1), 0x102844 1/1 (claude2), 0x11C1FC 100/100 (claude2), 0x13EFA0 100/100 (claude2), 0x0BC1E8 85/85 (claude2), 0x0BCC96 100/100 (claude2), 0x0BD308 48/48 (claude2), 0x0BD7E4 89/89 (claude2), 0x0BDDE2 1/1 (claude2), 0x0BDE42 5/5 (claude2), 0x0CB97E 1/1 (claude2), 0x0E1B36 6/6 (claude2), 0x0ECFAE 19/19 (claude2), 0x0F3582 24/24 (claude2), 0x10524A 2/2 (claude3), 0x11A8A8 100/100 (claude3), 0x12EF40 100/100 (claude3), 0x0BC404 1/1 (claude3), 0x0BC680 16/16 (claude3), 0x0BD2C2 64/64 (claude3), 0x0EA836 2/2 (claude3), 0x0ECF0C 22/22 (claude3), 0x0BBB18 80/80 (cw1), 0x0BC576 100/100 (cw1), 0x0EA28C 4/4 (cw1), 0x0ECF3E 100/100 (cw1) |
| 0x12EA9E / 0x12F29E / 0x0BB84C / 0x0BBD24 / 0x0BBDE0 / 0x0BBE2A / 0x0BC1CE / 0x0BC422 / 0x0BCEB0 / 0x0BCF30 / 0x0C9DEC / 0x0CAF90 / 0x10290C / 0x104F26 / 0x11C3A8 / 0x12EA24 / 0x0BB874 / 0x0CAFB8 / 0x0D9FC8 / 0x0DBDA2 / 0x0DC674 / 0x0E186A / 0x0ECFDA / 0x102A70 / 0x104EFA / 0x12CFD4 / 0x0BC2A0 / 0x0CB422 / 0x0DA0D6 / 0x0EA29E / 0x0EA418 | Obj_*_Tick (survey batch) | VERIFIED (MAME) | 0x12EA9E 4/4 (claude1), 0x12F29E 100/100 (claude1), 0x0BB84C 1/1 (claude1), 0x0BBD24 1/1 (claude1), 0x0BBDE0 1/1 (claude1), 0x0BBE2A 1/1 (claude1), 0x0BC1CE 1/1 (claude1), 0x0BC422 1/1 (claude1), 0x0BCEB0 24/24 (claude1), 0x0BCF30 36/36 (claude1), 0x0C9DEC 100/100 (claude1), 0x0CAF90 60/60 (claude1), 0x10290C 1/1 (claude2), 0x104F26 100/100 (claude2), 0x11C3A8 100/100 (claude2), 0x12EA24 1/1 (claude2), 0x0BB874 1/1 (claude2), 0x0CAFB8 63/63 (claude2), 0x0D9FC8 100/100 (claude2), 0x0DBDA2 18/18 (claude2), 0x0DC674 100/100 (claude2), 0x0E186A 6/6 (claude2), 0x0ECFDA 100/100 (claude2), 0x102A70 1/1 (claude3), 0x104EFA 2/2 (claude3), 0x12CFD4 100/100 (claude3), 0x0BC2A0 100/100 (claude3), 0x0CB422 100/100 (claude3), 0x0DA0D6 11/11 (claude3), 0x0EA29E 45/45 (claude3), 0x0EA418 2/2 (claude3) |
| 0x1087E8 / 0x1088F0 / 0x108936 / 0x0E598C / 0x0E5C2A / 0x0E5CBC / 0x0E6082 / 0x0E60EA / 0x0E6166 / 0x0E61AA / 0x0F5AFE / 0x0F5CF6 / 0x0F5D58 / 0x0BD3A4 / 0x0BD550 / 0x0BD9BE / 0x0BE16C / 0x0D740E / 0x0D747E / 0x0BD700 / 0x0BD990 / 0x0CB9EE / 0x0CBA34 / 0x0DC358 / 0x0DC486 / 0x0DC5B6 / 0x0CD504 / 0x0CD556 / 0x0CD5EA / 0x0CD646 / 0x0E077A / 0x0E101A / 0x0E1036 / 0x112C1A / 0x112C3C / 0x1134B2 / 0x1134BA / 0x113ED0 / 0x1180F8 / 0x118CF6 | Obj_*_Tick (survey batch) | VERIFIED (MAME) | 0x1087E8 100/100 (agy1), 0x1088F0 16/16 (agy1), 0x108936 100/100 (agy1), 0x0E598C 100/100 (agy2), 0x0E5C2A 10/10 (agy2), 0x0E5CBC 100/100 (agy2), 0x0E6082 23/23 (agy3), 0x0E60EA 100/100 (agy3), 0x0E6166 1/1 (agy3), 0x0E61AA 100/100 (agy3), 0x0F5AFE 100/100 (agy4), 0x0F5CF6 2/2 (agy4), 0x0F5D58 100/100 (agy4), 0x0BD3A4 64/64 (claude1), 0x0BD550 100/100 (claude1), 0x0BD9BE 100/100 (claude1), 0x0BE16C 64/64 (claude1), 0x0D740E 100/100 (claude1), 0x0D747E 46/46 (claude1), 0x0BD700 1/1 (claude2), 0x0BD990 1/1 (claude2), 0x0CB9EE 100/100 (claude2), 0x0CBA34 9/9 (claude2), 0x0DC358 2/2 (claude2), 0x0DC486 49/49 (claude2), 0x0DC5B6 1/1 (claude2), 0x0CD504 1/1 (claude3), 0x0CD556 40/40 (claude3), 0x0CD5EA 4/4 (claude3), 0x0CD646 67/67 (claude3), 0x0E077A 59/59 (claude3), 0x0E101A 3/3 (claude3), 0x0E1036 56/56 (claude3), 0x112C1A 2/2 (claude4), 0x112C3C 100/100 (claude4), 0x1134B2 1/1 (claude4), 0x1134BA 87/87 (claude4), 0x113ED0 88/88 (claude4), 0x1180F8 1/1 (cw1), 0x118CF6 1/1 (cw2) |
| 0x112D1A / 0x112D88 | Obj_*_Tick (survey batch) | VERIFIED (MAME) | 0x112D1A 1/1 (claude4), 0x112D88 87/87 (claude4) |
| 0x10AAC2 / 0x10AFBA / 0x10AFCC / 0x114800 / 0x114926 / 0x114A10 / 0x1094A6 | Obj_*_Tick (survey batch) | VERIFIED (MAME) | 0x10AAC2 100/100 (claude1), 0x10AFBA 8/8 (claude1), 0x10AFCC 75/75 (claude1), 0x114800 100/100 (claude3), 0x114926 2/2 (claude3), 0x114A10 2/2 (claude3), 0x1094A6 100/100 (claude4) |
| 0x1131A0 / 0x1134E6 / 0x113644 / 0x1140BE / 0x114108 / 0x114814 / 0x0E64DA / 0x0E6508 / 0x0E68C2 / 0x0E6906 / 0x0E6C36 / 0x0E6C64 / 0x0E6E0E / 0x0E6E54 / 0x13EE08 / 0x13EE50 / 0x0E7024 / 0x0E7036 / 0x0E7DEE / 0x0E7E1C / 0x0F0DB8 / 0x0F131C / 0x108E8E / 0x10AB30 / 0x10AC10 / 0x0E70CE / 0x0E7A7A / 0x0E7ABE / 0x0E81A0 / 0x0E8240 / 0x0E8494 / 0x11464E / 0x114984 / 0x114A76 / 0x109514 / 0x0DA64C / 0x0DA6C0 / 0x108D8E / 0x0DBF16 / 0x0DBF70 | Obj_*_Tick (survey batch) | VERIFIED (MAME) | 0x1131A0 1/1 (agy1), 0x1134E6 100/100 (agy1), 0x113644 1/1 (agy1), 0x1140BE 7/7 (agy1), 0x114108 100/100 (agy1), 0x114814 1/1 (agy1), 0x0E64DA 1/1 (agy3), 0x0E6508 1/1 (agy3), 0x0E68C2 1/1 (agy3), 0x0E6906 100/100 (agy3), 0x0E6C36 1/1 (agy3), 0x0E6C64 28/28 (agy3), 0x0E6E0E 1/1 (agy3), 0x0E6E54 31/31 (agy3), 0x13EE08 30/30 (agy4), 0x13EE50 100/100 (agy4), 0x0E7024 2/2 (agy4), 0x0E7036 100/100 (agy4), 0x0E7DEE 2/2 (agy4), 0x0E7E1C 30/30 (agy4), 0x0F0DB8 100/100 (agy4), 0x0F131C 7/7 (agy4), 0x108E8E 100/100 (claude1), 0x10AB30 8/8 (claude1), 0x10AC10 83/83 (claude1), 0x0E70CE 100/100 (claude2), 0x0E7A7A 1/1 (claude2), 0x0E7ABE 100/100 (claude2), 0x0E81A0 20/20 (claude2), 0x0E8240 100/100 (claude2), 0x0E8494 100/100 (claude2), 0x11464E 100/100 (claude3), 0x114984 66/66 (claude3), 0x114A76 35/35 (claude3), 0x109514 8/8 (claude4), 0x0DA64C 1/1 (claude4), 0x0DA6C0 1/1 (claude4), 0x108D8E 100/100 (cw1), 0x0DBF16 5/5 (cw3), 0x0DBF70 99/99 (cw3) |
| 0x118D7C / 0x118DDC / 0x0F0788 / 0x0F0814 / 0x0F0846 / 0x0F0D7E / 0x0F0D8A | Obj_*_Tick (survey batch) | VERIFIED (MAME) | 0x118D7C 1/1 (agy2), 0x118DDC 86/86 (agy2), 0x0F0788 100/100 (agy2), 0x0F0814 20/20 (agy2), 0x0F0846 100/100 (agy2), 0x0F0D7E 100/100 (agy2), 0x0F0D8A 7/7 (agy2) |
| 0x0CB97E / 0x0BDE92 | graphics RAM writes completed (0x55C8, 0x5A86, 0x53B2, 0x176832, 0x1768CA) | VERIFIED (MAME, gfx RAM compared) | 1/1, 32/32 (claude3) |
| 0x108DFC | Obj_*_Tick (survey batch) | VERIFIED (MAME) | 0x108DFC 8/8 (claude4) |
| 0x0CC11C / 0x0F4D52 / 0x0F4DDE / 0x0F4E1E / 0x0D9D9E / 0x0D9E06 | Obj_*_Tick (survey batch) | VERIFIED (MAME) | 0x0CC11C 6/6 (claude1), 0x0F4D52 100/100 (claude2), 0x0F4DDE 3/3 (claude2), 0x0F4E1E 100/100 (claude2), 0x0D9D9E 15/15 (claude4), 0x0D9E06 100/100 (claude4) |
| 0x0BE262 / 0x0BD308 / 0x0BDDE2 / 0x0BD7E4 / 0x0BDE42 / 0x0BD2C2 / 0x0BDDB8 | graphics RAM writes completed (agy hydrogen); 0x0D0678/0x0D0690 copies fixed to store the low byte at +0x1C (codewhale review) | VERIFIED (MAME, gfx RAM compared) | 100/100, 48/48, 1/1, 89/89, 5/5, 64/64, 1/1 |
| 0x0EB2B0 / 0x0EB2BC / 0x0EB2EA / 0x0EBA0E / 0x0EEB76 / 0x0EEB82 / 0x0EEBB0 / 0x17B056 / 0x17B05E / 0x18764E / 0x18765E / 0x187938 | Obj_*_Tick (survey batch) | VERIFIED (MAME) | 0x0EB2B0 60/60 (claude1), 0x0EB2BC 12/12 (claude1), 0x0EB2EA 60/60 (claude1), 0x0EBA0E 12/12 (claude1), 0x0EEB76 60/60 (claude1), 0x0EEB82 8/8 (claude1), 0x0EEBB0 60/60 (claude1), 0x17B056 1/1 (claude2), 0x17B05E 60/60 (claude2), 0x18764E 1/1 (claude4), 0x18765E 10/10 (claude4), 0x187938 60/60 (claude4) |
| 0x17A8D0 / 0x17A8E0 / 0x17AAEE / 0x17AAFE / 0x17C05E / 0x17E000 / 0x17E05A / 0x17E108 / 0x17E6B6 / 0x184184 | Obj_*_Tick (survey batch) | VERIFIED (MAME) | 0x17A8D0 6/6 (claude2), 0x17A8E0 60/60 (claude2), 0x17AAEE 2/2 (claude2), 0x17AAFE 60/60 (claude2), 0x17C05E 60/60 (claude3), 0x17E000 4/4 (claude3), 0x17E05A 4/4 (claude3), 0x17E108 2/2 (claude3), 0x17E6B6 6/6 (claude3), 0x184184 8/8 (claude3) |
| 0x0CC05A / 0x0CCD04 / 0x0CCE6E / 0x0CD256 / 0x0CD412 / 0x0DA82E / 0x0DC77A / 0x0DDAE0 / 0x0DE064 / 0x0DE0B6 / 0x0DE140 / 0x0DE18C / 0x0DE214 / 0x0DE794 / 0x0DE7E0 / 0x0DE86A / 0x0DE8B0 / 0x0E2124 / 0x0E2170 / 0x0E226C / 0x0E2288 / 0x0EBA96 / 0x0EBB12 / 0x0EC65E / 0x0ED124 / 0x0ED156 / 0x0F28EA / 0x0F2964 / 0x0FF990 / 0x1138BC / 0x113B74 / 0x113BC2 / 0x118358 / 0x118486 / 0x118494 / 0x118602 / 0x1186C4 / 0x1187DE / 0x118E88 / 0x118EDE / 0x119062 / 0x119090 / 0x119176 / 0x1191A6 / 0x1191FA / 0x11922A / 0x1192A0 / 0x1192AE / 0x119360 / 0x11946C / 0x1194C2 / 0x1199A8 / 0x1199CC / 0x119B40 / 0x119BBE / 0x119D06 / 0x119D36 / 0x12CB72 / 0x12CC44 / 0x12D0AE / 0x12D0C4 / 0x12D0E6 / 0x12D10C / 0x12D140 / 0x12D1C6 / 0x12D1E0 / 0x12D22C / 0x12D284 / 0x12D2B4 / 0x12D2D4 / 0x12D2FC / 0x12D350 / 0x12D36C / 0x12D382 / 0x12D3FA / 0x12D414 / 0x12D446 / 0x12D466 / 0x12D482 / 0x12D49E / 0x12D4BA / 0x13029E / 0x1302B8 / 0x1302F8 / 0x130360 / 0x1307E0 / 0x131414 / 0x114092 / 0x126B3E / 0x0CEB3E / 0x0D76F4 / 0x0D78C0 / 0x130044 / 0x131408 / 0x0CEF44 | Obj_*_Tick (survey batch) | VERIFIED (MAME) | 0x0CC05A 2/2 (claude1), 0x0CCD04 41/41 (claude1), 0x0CCE6E 60/60 (claude1), 0x0CD256 60/60 (claude1), 0x0CD412 60/60 (claude1), 0x0DA82E 1/1 (claude1), 0x0DC77A 60/60 (claude1), 0x0DDAE0 60/60 (claude1), 0x0DE064 48/48 (claude1), 0x0DE0B6 60/60 (claude1), 0x0DE140 48/48 (claude1), 0x0DE18C 60/60 (claude1), 0x0DE214 60/60 (claude1), 0x0DE794 60/60 (claude1), 0x0DE7E0 60/60 (claude1), 0x0DE86A 60/60 (claude1), 0x0DE8B0 60/60 (claude1), 0x0E2124 43/43 (claude2), 0x0E2170 60/60 (claude2), 0x0E226C 1/1 (claude2), 0x0E2288 60/60 (claude2), 0x0EBA96 60/60 (claude2), 0x0EBB12 16/16 (claude2), 0x0EC65E 60/60 (claude2), 0x0ED124 22/22 (claude2), 0x0ED156 60/60 (claude2), 0x0F28EA 60/60 (claude2), 0x0F2964 9/9 (claude2), 0x0FF990 60/60 (claude2), 0x1138BC 60/60 (claude3), 0x113B74 30/30 (claude3), 0x113BC2 60/60 (claude3), 0x118358 60/60 (claude3), 0x118486 32/32 (claude3), 0x118494 60/60 (claude3), 0x118602 16/16 (claude3), 0x1186C4 16/16 (claude3), 0x1187DE 60/60 (claude3), 0x118E88 60/60 (claude3), 0x118EDE 60/60 (claude3), 0x119062 60/60 (claude3), 0x119090 60/60 (claude3), 0x119176 57/57 (claude3), 0x1191A6 24/24 (claude3), 0x1191FA 60/60 (claude3), 0x11922A 24/24 (claude3), 0x1192A0 24/24 (claude3), 0x1192AE 34/34 (claude3), 0x119360 60/60 (claude4), 0x11946C 24/24 (claude4), 0x1194C2 60/60 (claude4), 0x1199A8 60/60 (claude4), 0x1199CC 60/60 (claude4), 0x119B40 60/60 (claude4), 0x119BBE 60/60 (claude4), 0x119D06 60/60 (claude4), 0x119D36 60/60 (claude4), 0x12CB72 1/1 (claude4), 0x12CC44 30/30 (claude4), 0x12D0AE 10/10 (claude4), 0x12D0C4 60/60 (claude4), 0x12D0E6 1/1 (claude4), 0x12D10C 2/2 (claude4), 0x12D140 15/15 (claude4), 0x12D1C6 30/30 (cw1), 0x12D1E0 60/60 (cw1), 0x12D22C 38/38 (cw1), 0x12D284 38/38 (cw1), 0x12D2B4 60/60 (cw1), 0x12D2D4 60/60 (cw1), 0x12D2FC 30/30 (cw1), 0x12D350 30/30 (cw1), 0x12D36C 60/60 (cw1), 0x12D382 60/60 (cw2), 0x12D3FA 60/60 (cw2), 0x12D414 60/60 (cw2), 0x12D446 60/60 (cw2), 0x12D466 40/40 (cw2), 0x12D482 40/40 (cw2), 0x12D49E 60/60 (cw2), 0x12D4BA 60/60 (cw2), 0x13029E 1/1 (cw2), 0x1302B8 60/60 (cw2), 0x1302F8 4/4 (cw3), 0x130360 1/1 (cw3), 0x1307E0 60/60 (cw3), 0x131414 60/60 (cw3), 0x114092 2/2 (cw4), 0x126B3E 60/60 (cw4), 0x0CEB3E 60/60 (cw4), 0x0D76F4 60/60 (cw4), 0x0D78C0 60/60 (cw4), 0x130044 1/1 (strata), 0x131408 1/1 (strata), 0x0CEF44 4/4 (strata) |
| 0x11949A / 0x131616 / 0x13DE92 / 0x13DF4C | Obj_*_Tick (survey batch) | VERIFIED (MAME) | 0x11949A 54/54 (claude4), 0x131616 45/45 (cw5), 0x13DE92 1/1 (cw5), 0x13DF4C 60/60 (cw5) |
| 0x1193E2 / 0x119422 / 0x13E12C | Obj_*_Tick (survey batch) | VERIFIED (MAME) | 0x1193E2 60/60 (claude4), 0x119422 6/6 (claude4), 0x13E12C 1/1 (cw5) |
Note on `Obj_Spawn` argument order: the word pushed last (just before the fn long) is the group, the one before it is ngroups. `tests/test_objects` accepts `F3_ONLY=<substring>` to run a subset.
| 0x0D5464 | Obj_D5464_Tick | VERIFIED (MAME) | 150/150 incl. 5 state changes (hermes draft; fixed to use the RAM RNG seed via `Rng_NextMem`) |
| 0x0CEE7C / 0x0CEE84 / 0x0CEE8C | Obj_Dying1/2/3_Tick | VERIFIED (MAME) | 8/8 each; frees the object on the 3rd frame |

Tools: `scripts/capture_obj.sh <addr>` (tries the savestates until enough ticks), `scripts/integrate_obj.sh <addr> <draft.c>` (header + Makefile + test entry). Shared `Rng_NextMem()` in `f3mem.h` (seed in RAM 0x406BAA).

Delegation workflow: `scripts/export_bundle.sh <addr>` writes `work/disasm/bundle_<addr>.s` (flow-following disassembly incl. callees and PC-relative jump tables, stopping at routines declared in `src/**/*.h`) → agy drafts the C → `F3_SPR=1 F3_COND="a1==<addr>" scripts/capture_fn.sh 104EE 104F0 300 work/ram/obj_<ADDR>` → `tests/test_objects.c`.

## Ghidra coverage
- `scripts/ghidra/obj_states.py` discovers object tick routines referenced only as immediates (`move.l #fn,(A0)`, `move.l #fn,-(SP)` before `jsr Obj_Spawn`) plus the ones observed in MAME (`traces/obj_ticks.txt`, 58 addresses from pool-runner logs). Clean bootstrap: 583 → 2527 functions; 58/58 observed tick routines are functions.

## Verification tooling
- `scripts/capture_fn.sh <entry> <exit> <n> <prefix>`: MAME debugger breakpoints (headless `-debug -debugger none`) dump Work RAM + registers exactly at a code region's entry/exit → golden pre/post pairs for any routine. Options: `F3_DPRAM` (sound DPRAM), `F3_SWEEP`/`F3_SWEEP_BASE` (force the word arg), `F3_POKE32` (force RAM at entry), `F3_LOG` (log registers at inner PCs, e.g. indirect calls), `F3_EXIT=ret` (post at the caller's return address; abandoned calls are dropped), `F3_MAXFRAMES` (watchdog: a stopped `-debug` MAME ignores SIGTERM, use `timeout -s KILL`), `F3_COND` (conditional entry, e.g. `a1==102fe6` at the pool call site 0x104EE to capture one object routine), `F3_SPR` (sprite RAM), `F3_POKEA0B` (byte pokes relative to A0), `F3_POKEA0P` (`ptr:off=val`: byte pokes at `rd32(A0+ptr)+off`), `F3_POKEFN` (apply the A0 pokes only to calls of that fn and do not capture them: force a parent while capturing its children). Tests share `tests/capture.h`.

## Known caveats
- Ghidra auto-analysis creates pointers over code reached only via `pea` (task entries); `fix_flow.py` must run after `apply_symbols.py` (bootstrap does this).
- Struct types (`symbols/dariusg_types.tsv`, `apply_types.py`) give field names in pseudo-C; `extraout_A0` noise remains because callees are not declared to preserve A0.
- Input timing: the object tick of frame t+1 sees io_ch polled at the start of that frame (test_player variant `RAM_t+1`).
- Auto-analysis made scalar constants into refs to SOUND_BANK/PIVOT_RAM/WORK_RAM_MIRROR (1 ref per function) — likely false XREFs, review before trusting.
- The repo's `ghidra_proj/taito_f3` is locked by Hermes' pyghidra-mcp while the gateway runs; bootstrap there once it is stopped.
