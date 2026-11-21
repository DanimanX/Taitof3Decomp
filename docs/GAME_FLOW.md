<!-- Provenance: drafted by agy (Gemini 4 Neon) from work/disasm/task0_10000_10700.s
     (after the fix_flow repair) and work/decomp, reviewed by Claude 2026-11-21.
     Independently verified: the 4 spawns + kill masks (0x7FFE keeps slot 15),
     CACR enable at 0x10210, sound cmds 0x94/0x95, strings at 0x1047A/0x10402..,
     'TAITO' EEPROM magic in 0x50DC, free-play/auto-start bits in 0x4D06,
     test-switch check at 0x10420. Everything else: reviewed, not re-derived.
     Canonical names: symbols/dariusg.tsv. -->

# Darius Gaiden — `Task0_Main` (`0x010172`) Top-Level Game Flow

> Base register `A5 = 0x408000`. Note: `TRAP #1` expects `(entry.l, slot.l, arg.l)` on stack; spawns at `0x10262`, `0x10272`, `0x1028A`, `0x1030A` push only 8 bytes (`slot`, `entry`), passing no explicit `arg` (`arg = none`).

## 1. `Task0_Main` Control Flow (Pseudo-Code)

```text
// === Cold-Boot & Sound Init (0x10172..0x10235) ===
 1. [0x10172] TRAP #5 ()                                      // Sys_Yield: wait 1 frame
 2. [0x10174] call 0x005A3A()                                 // Load font tiles ROM 0x1120A -> CHAR_RAM
 3. [0x1017A] if (call 0x0050DC() != 0) goto TEST_MODE        // [0x10182] EEPROM verify/init failed -> 0x10432
 4. [0x10186] call 0x005666("WAIT A MOMENT"@0x1047A, 0x61C79C, attr=7) // Print boot message to TEXT_RAM
 5. [0x101A0] TRAP #4 (frames=0x00B4)                         // Sys_Sleep(180 frames ~= 3s)
 6. [0x101AA] call 0x13F238(0x0000.w); call 0x0039D8(0x0000.w)// Sound pre-hook + command 0x0000
 7. [0x101BE] call 0x13F238(0x0094.w); call 0x0039D8(0x0094.w)// Sound pre-hook + command 0x0094
 8. [0x101D2] [0x406BB6] = (!bit4(io_ch[0].cur) << 7)         // Latch boot-time active-low test/service
                         | (!bit3(io_ch[0].cur) << 6)         // switches from 0x40221C into bits 7..6
 9. [0x101F2] TRAP #4 (frames=0x00B4)                         // Sys_Sleep(180 frames ~= 3s)
10. [0x101FC] call 0x13F238(0x0095.w); call 0x0039D8(0x0095.w)// Sound pre-hook + command 0x0095
11. [0x10210] CACR = 1                                        // Enable 68020 instruction cache
12. [0x10216] call 0x003CBA(vol, vol, vol, vol)               // Push 0x402240.w x4 -> SOUND_DPRAM 0xC007F8..FB
13. [0x10230] call 0x005880()                                 // Clear TEXT_RAM ("WAIT A MOMENT") & ext scroll

// === Main Attract <-> Gameplay Outer Loop (LAB_00010236) ===
14. LOOP [0x10236]:
15.   [0x10236] call 0x005A3A()                               // Reload CHAR_RAM font
16.   [0x1023C] [0x406BB4] = 0; [0x406BB5] = 0; no_idle = 0   // Clear game flags & kernel no_idle (0x406BAE)
17.   [0x10248] call 0x003C24(); call 0x001070(); call 0x00117E() // Stop audio; init/clear SPRITE_RAM
18.   [0x10256] call 0x005A12(); call 0x005B0C(); call 0x0014DA() // Clear all layers; init LINE_RAM; load palettes
19.   [0x10262] TRAP #1 (entry=0x010488, slot=4,   arg=none)  // Spawn object pool runner (groups 0..10)
20.   [0x10272] TRAP #1 (entry=0x0038AC, slot=0xF, arg=none)  // Spawn persistent slot-15 task
21.   [0x10282] TRAP #5 ()                                    // Sys_Yield: let slot 4 & 15 init
22.   [0x10284] [0x406BBC] = [0x40223B]                       // Set sound_mute_flag = attract_sound_mute
23.   [0x1028A] TRAP #1 (entry=0x0B8404, slot=1,   arg=none)  // Spawn attract/title task in slot 1

// --- Attract-Mode Monitor Loop (LAB_0001029a) ---
24.   ATTRACT_WAIT [0x1029A]:
25.     [0x1029A] TRAP #5 ()                                  // Sys_Yield
26.     [0x1029C] call 0x01033C()                             // Test-switch check (may jump to 0x10432), credit HUD, latch flags
27.     [0x102A0] if ([0x40223D] != 0 && (rng_seed.hi16 & 0x7FF) == 0) goto START_GAME
28.     [0x102B4] if ([0x40223F] != 0) {                      // Free Play enabled
29.       [0x102BC]   [0x409D38] = 0; if (!bit7(0x402225)) goto START_GAME // P1 Start pressed (active-low)
30.       [0x102CC]   if ([0x40223A] == 0) {                  // 2P allowed
31.       [0x102D2]     [0x409D38] = 1; if (!bit7(0x402229)) goto START_GAME // P2 Start pressed
32.                   }
33.       [0x102E2]   goto ATTRACT_WAIT
34.               } else {                                    // Coin mode
35.       [0x102E4]   if ((int8_t)call 0x000FEC() < 0) goto ATTRACT_WAIT // Stay until credit/coin ready
36.               }

// --- Launch Gameplay & Monitor (LAB_000102ee) ---
37.   START_GAME [0x102EE]:
38.     [0x102EE] call 0x003C24(); call 0x00117E(); call 0x005A12() // Stop audio; clear sprites & tilemaps
39.     [0x10300] TRAP #2 (mask=0x00007FFE)                   // Sys_KillMask: kill slots 1..14 (keep 0 & 15)
40.     [0x1030A] TRAP #1 (entry=0x004146, slot=1, arg=none)  // Spawn main game task in slot 1
41.     [0x1031A] [0x406BBC] = 0                              // Unmute sound for gameplay
42.   GAME_WAIT [0x10320]:
43.     [0x10320] TRAP #5 ()                                  // Sys_Yield
44.     [0x10322] call 0x01033C()                             // Test-switch check, credit HUD, latch flags
45.     [0x10326] if (([0x406BB5] & 1) == 0) goto GAME_WAIT   // Wait for game-over bit 0
46.     [0x1032E] TRAP #2 (mask=0x0000FFFE)                   // Sys_KillMask: kill slots 1..15 (keep 0)
47.     [0x10338] goto LOOP                                   // Re-init & return to attract

// === Test Mode Entry (LAB_00010432, via 0x10182 or 0x10420 switch edge) ===
48. TEST_MODE [0x10432]:
49.   [0x10432] TRAP #2 (mask=0x0000FFFE)                     // Sys_KillMask: kill slots 1..15
50.   [0x1043C] TRAP #1 (entry=0x1A1A26, slot=1, arg=*(u32*)0x00000000) // Spawn Test Mode menu task
51.   [0x10452] [0x402239] = 0; io_out_4a0004 = 0; [0x406BBC] = 0 // Disable coin sys/lockout; unmute sound
52.   [0x1045E] call 0x00117E(); call 0x005A12(); call 0x005B0C(); call 0x0014DA(); call 0x003C24()
53.   [0x10474] while (true) TRAP #5 ()                       // Yield forever until Test Mode resets
```

---

## 2. Spawned Tasks

| Entry Addr | Slot | Arg | Spawner (PC) | Proposed Name & Purpose | Status | Evidence |
|---|---|---|---|---|---|---|
| `0x010172` | `0` | `0` | `Kernel_ColdBoot` (`0x001DF0`) | `Task0_Main`: Top-level boot, attract/game/test-mode supervisor | **CONFIRMED** | `docs/KERNEL.md`, `task0_10000_10700.s` |
| `0x010488` | `4` | none | `Task0_Main` (`0x01026E`) | `Task_ObjectPoolRunner`: Clears (`0x10638` + `0x410060..0x4194DF`) and ticks 11×32 `0x6C`-byte objects per frame unless `0x406BB5` bit 1 set | **CONFIRMED** | Decoded opcodes `0x010488`–`0x010500` in `task0_10000_10700.s` |
| `0x0038AC` | `15` (`0xF`) | none | `Task0_Main` (`0x01027E`) | `Task_PersistentSys`: Background daemon kept alive across attract→game (`0x7FFE` preserves slot 15; near sound driver `0x3960`) | **GUESS** | Not in `work/decomp/INDEX.tsv`; lifecycle at `0x10272`/`0x10300`/`0x1032E` |
| `0x0B8404` | `1` | none | `Task0_Main` (`0x010296`) | `Task_AttractMode`: Attract/title/demo sequencer; killed on coin/start | **GUESS** | Not in `work/decomp/INDEX.tsv`; runs during `0x1029A`–`0x102EC`, killed at `0x10300` |
| `0x004146` | `1` | none | `Task0_Main` (`0x010316`) | `Task_GameMain`: Active credit-start/gameplay controller until `0x406BB5` bit 0 | **GUESS** | Not standalone in `work/decomp/INDEX.tsv` (inside `0x4088` range); lifecycle `0x1030A`–`0x1032E` |
| `0x1A1A26` | `1` | `*(u32*)0x0` | `Task0_Main`/`0x10420` (`0x01044C`) | `Task_TestModeMenu`: Service/test mode main menu (`"TESTMODE MAIN MENU"`, `"  MONITOR TEST"`) | **CONFIRMED** | `work/decomp/1A1A26_FUN_001a1a26.c` |

---

## 3. Functions Called by `Task0_Main` (and Sub-Helpers)

| Addr | Proposed Name | One-Line Purpose | Evidence | Status |
|---|---|---|---|---|
| `0x005A3A` | `Gfx_LoadCharFont` | Copies `0x800` word-swapped longs from ROM `0x1120A` to `CHAR_RAM` | `005A3A_FUN_00005a3a.c` | **CONFIRMED** |
| `0x0050DC` | `Eeprom_VerifyOrInit` | Validates `"TAITO"` magic (`0x406C54`), region (`0x1FFFFE`), checksums; resets & prompts `"PUSH TEST SWITCH"` on error, returns `-1` | `0050DC_FUN_000050dc.c` | **CONFIRMED** |
| `0x005666` | `Text_DrawString` | Writes null-terminated ASCII string to `TEXT_RAM` with `(attr<<1)` high byte | `005666_FUN_00005666.c` | **CONFIRMED** |
| `0x13F238` | `Sound_PreCmdHook` | 36-byte Work RAM hook always invoked before `0x39D8`/`0x3B66` with same word arg on stack (decomp empty) | `INDEX.tsv`, `13F238`, `000FA8`, `13DAD4`, `13F206` | **GUESS** |
| `0x0039D8` | `Sound_PlayCmd` | Dispatches sound ID via table `0x406BB8` into `SOUND_DPRAM` (`0xC00000`, ptr `0xC00480`) if `0x406BBC == 0` | `0039D8`, `sweepA_io_audio.log` (`0039FC`/`003B58`/`003A1C`) | **CONFIRMED** |
| `0x003CBA` | `Sound_SetVolume` | Writes 4 volume bytes to `SOUND_DPRAM` `0xC007F8..0xC007FB` | `003CBA`, `1A043C`, `sweepA_io_audio.log` (`003CC8..DA`, `$30`) | **CONFIRMED** |
| `0x005880` | `Gfx_ClearTextLayer` | Zeroes `0x800` longs of `TEXT_RAM` and resets `ext_scroll_x/y` & `0x402278`/`0x40227C` | `005880_FUN_00005880.c` | **CONFIRMED** |
| `0x003C24` | `Sound_StopAll` | Stops all active sound slots (`0x406BBE[0x80]`) and resets channel params (`0x408817`) via `0x3998` -> `SOUND_DPRAM` | `003C24`, `003998`, `sweepA_io_audio.log` (`003C32`/`003CB4`) | **CONFIRMED** |
| `0x001070` | `Sprite_InitBuffers` | Initializes both `SPRITE_RAM` banks (`0x600000`/`0x608000`) headers, flip offsets, and end sentinels | `001070_FUN_00001070.c` | **CONFIRMED** |
| `0x00117E` | `Sprite_ClearAll` | Clears all `0x3FC` sprite entries in both `SPRITE_RAM` banks and resets list pointers (`0x407354..68`) | `00117E_FUN_0000117e.c` | **CONFIRMED** |
| `0x005A12` | `Gfx_ClearAllLayers` | Clears `TEXT_RAM` (`0x5880`) and all 4 `PF_RAM` playfields + line-scroll/scroll shadows (`0x58DA`/`5918`/`5956`/`59B4`) | `005A12` and callees | **CONFIRMED** |
| `0x005B0C` | `Gfx_InitLineRam` | Sets flip line offset `0x4022AA`, fills `LINE_RAM` defaults/priorities (`0x620000..75FE`) | `005B0C_FUN_00005b0c.c` | **CONFIRMED** |
| `0x0014DA` | `Pal_LoadDefault` | Loads default palette blocks into `PALETTE_RAM` via `0x14E8`/`0x1504` -> `0x1520` | `0014DA`, `0014E8`, `001504`, `001520` | **CONFIRMED** |
| `0x000FEC` | `Coin_CheckStartReady` | Returns `0xFF` (<0) if no credit/partial-coin ready; `0` or remaining coins (>=0) when credit/coin present | `000FEC`, `000DFA`, `000ECA` | **CONFIRMED** |
| `0x01033C` | `Task0_PerFrameUpdate` | Calls `0x10420` (test switch), `0x10352` (credit HUD), latches `0x406BB4->0x406BB5`, clears `0x406BB4` bit 7 | `task0_10000_10700.s` (`01033C`–`010350`) | **CONFIRMED** |
| `0x010352` | `Hud_DrawCreditStatus` | Draws blinking `"FREE PLAY"`, `"  ENJOY  "` (if `0x1FFFFE==2`, `0x406BB5` bit 7, `0x1016()==0`), or `"CREDIT   "` + count (`0x5690`) at `0x61CE3E` | `task0_10000_10700.s` (`010352`–`01041F`) | **CONFIRMED** |
| `0x010420` | `Sys_CheckTestSwitch` | Tests `io_ch[0].pressed` (`0x40221D`) bits 4 & 3; if pressed, kills `0xFFFE`, spawns `0x1A1A26`, yields forever | `task0_10000_10700.s` (`010420`–`010478`) | **CONFIRMED** |
| `0x001016` | `Coin_CoinsNeeded` | Returns `0` when credits (`0x400090`) >= 1 or coins meet `0x4000A2`, else remaining coins / `0xFF` | `001016_FUN_00001016.c` | **CONFIRMED** |
| `0x005690` | `Text_DrawNumber` | Renders numeric value (`0x400090` credits) to `TEXT_RAM` (`0x61CE4C`); Ghidra decomp failed, confirmed by callers | `0103B4`–`0103CE`, `0B80A4`, `0B7B52` | **CONFIRMED** |

---

## 4. Work RAM Globals Touched by `Task0_Main` (and `0x1033C`/`0x10352`/`0x10420`)

| Abs Addr | A5 Offset | Size | Proposed Name | Evidence |
|---|---|---|---|---|
| `0x400090` | `-0x7F70` | 2 B | `credit_count` | **CONFIRMED**: read at `0x103B4` & printed after `"CREDIT   "`; incremented/capped at 9 in `0x0DFA`/`0x0ECA` |
| `0x40221C` | `-0x5DE4` | 1 B | `io_ch[0].cur` | **CONFIRMED** (`symbols/dariusg.tsv`): bits 4 & 3 (test/service switches) sampled at boot `0x101D6`/`0x101E4` |
| `0x40221D` | `-0x5DE3` | 1 B | `io_ch[0].pressed` | **CONFIRMED** (`symbols/dariusg.tsv`): bits 4 & 3 enter Test Mode at `0x10420`/`0x10428` (and `0x50DC`) |
| `0x402225` | `-0x5DDB` | 1 B | `io_ch[2].pressed` | **CONFIRMED** (`symbols/dariusg.tsv`): bit 7 = P1 Start edge in Free Play at `0x102C2` |
| `0x402229` | `-0x5DD7` | 1 B | `io_ch[3].pressed` | **CONFIRMED** (`symbols/dariusg.tsv`): bit 7 = P2 Start edge in Free Play at `0x102D8` |
| `0x402234` | `-0x5DCC` | 1 B | `io_out_4a0004` | **CONFIRMED** (`symbols/dariusg.tsv`): cleared at `0x10456` on Test Mode entry (coin lockout) |
| `0x402239` | `-0x5DC7` | 1 B | `coin_sys_enable` | **CONFIRMED**: set `0xFF` in `Reset` (`0x100AE`), cleared at `0x10452`; gates coin checks/lockout/SFX in `0x0DFA`/`0x0FA8` |
| `0x40223A` | `-0x5DC6` | 1 B | `cabinet_1p_only` | **CONFIRMED** nonzero skips P2 Start at `0x102CC` (EEPROM unpacked in `0x4D06`); **GUESS** 1P-only cabinet setting |
| `0x40223B` | `-0x5DC5` | 1 B | `attract_sound_mute` | **CONFIRMED**: `(0x406C5E & 0x40)==0` in `0x4D06`; copied to `0x406BBC` during attract (`0x10284`) |
| `0x40223D` | `-0x5DC3` | 1 B | `auto_start_flag` | **CONFIRMED** EEPROM `0x406C6C` bit 30 (`0x4D06`), triggers random attract→game at `0x102A0`–`0x102B0`; **GUESS** burn-in/auto-play |
| `0x40223F` | `-0x5DC1` | 1 B | `free_play_flag` | **CONFIRMED**: EEPROM `0x406C6C` bit 31 (`0x4D06`); enables `"FREE PLAY"` (`0x10352`) and P1/P2 start without coins (`0x102B4`) |
| `0x402240` | `-0x5DC0` | 2 B | `sound_master_vol` | **CONFIRMED**: EEPROM `0x406C62` (`0x4D06`), pushed 4× to `0x3CBA` (`0x10216`–`0x10226`), edited `0..0x3F` in `0x1A043C` |
| `0x4022AD` | `-0x5D53` | 1 B | `frame_counter.lo` | **CONFIRMED** (low byte of `0x4022AC` `frame_counter`): bit 2 toggles `"FREE PLAY"` palette 7/8 at `0x103DA` |
| `0x406BAA` | `-0x1456` | 2 B | `rng_seed.hi16` | **CONFIRMED** (`symbols/dariusg.tsv`): masked `& 0x7FF` at `0x102A8`–`0x102B0` for `0x40223D` auto-start |
| `0x406BAE` | `-0x1452` | 1 B | `no_idle` | **CONFIRMED** (`symbols/dariusg.tsv`): cleared at `0x10244` on outer loop start |
| `0x406BB4` | `-0x144C` | 1 B | `game_flags_next` | **CONFIRMED**: cleared `0x1023C`, copied to `0x406BB5` with bit 7 auto-cleared at `0x10344`–`0x1034A`; written by stage tasks (`0x13F5B8`) |
| `0x406BB5` | `-0x144B` | 1 B | `game_flags` | **CONFIRMED**: latched from `0x406BB4` at `0x10344`; bit 0 = game-over (`0x10326`), bit 1 = pause objs/waits (`0x1048E`/`0x13DAD4`), bit 7 = `"ENJOY"` (`0x1035A`) |
| `0x406BB6` | `-0x144A` | 1 B | `boot_sys_switches` | **CONFIRMED**: cleared `0x101D2`; bits 7 & 6 record active-low boot state of `io_ch[0].cur` bits 4 & 3 (`0x101D6`–`0x101EC`) |
| `0x406BBC` | `-0x1444` | 1 B | `sound_mute_flag` | **CONFIRMED**: gates `0x39D8`; set from `0x40223B` in attract (`0x10284`), cleared for gameplay (`0x1031A`) & Test Mode (`0x1045A`) |
| `0x409D38` | `+0x1D38` | 2 B | `start_player_id` | **CONFIRMED**: set to `0` (P1) at `0x102BC` or `1` (P2) at `0x102D2` when starting from Free Play |
