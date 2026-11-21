<!-- Provenance: drafted by agy (Gemini 4 Neon) from work/disasm/player_C63E8.s +
     work/decomp, reviewed by Claude 2026-11-21. Verified in MAME/ROM:
     players[0]=0x41B190 (pos_x 0x41B1A4 moves 3 px/frame, clamp -8 at 0xC69DC,
     Up -> angle 0xC0, Y 14..210), score +0x48 = 0x41B1D8 (screen 3400/4600/9000,
     writer PC 0xD0E1A), slot 3 = collision task (its D1/D3 at yield = hitbox
     min X/Y, which is what the first RAM diff picked up). Other claims reviewed,
     not re-derived. Canonical names: symbols/dariusg.tsv. -->

# Darius Gaiden — `Task_Player1Ship` (`0xC63E8`–`0xC6A63`) Analysis

> **Key Finding (reconciling MAME):** `Task_Player1Ship` (`0xC63E8`, kernel slot 3) is actually the **Per-Frame Collision & Terrain Scroll-Sync Task** (**CONFIRMED**). It does **not** poll controls, move the ship, spawn shots, or run death/respawn directly. At `0xC63F0` (`TRAP #5`, resuming at `0xC63F2`), `D1.w`/`D3.w` hold P1's hitbox min-X/min-Y (`pos_x + hb_x0`, `pos_y + hb_y0`) solely as **leftover registers** from `SUB_000c64fc` (when P2 is inactive), while `A0=A1=0x41A988`, `A2=0x41ACF0`, `A3=0x41ACE0`, `A4=0x4143E0` are leftover collision-list/object pointers (**CONFIRMED**). Actual **ship joystick movement, terrain collision, bounds clamping, and banking** in this range live in object-pool callback `FUN_000c6866` → `FUN_000c691a` (`0xC685A`–`0xC6A62`), ticked by `Task_ObjectPoolRunner` (slot 4) after spawn from `Task_C6A64` (`0xC6A88`, slot 5) (**CONFIRMED**).

---

## 1. Main Loop & Player Movement Pseudo-Code

### A. `Task_Player1Ship` (`0xC63E8`–`0xC6409`, Kernel Slot 3) — **CONFIRMED**
```text
 1. [0C63E8] [0x409D1C] = 0                          // Enable terrain collision (sf)
 2. [0C63EC] call 0x0C640A()                         // Reset collision list cursors & bomb_dmg[2]
 3. LOOP [0C63F0]:
 4.   [0C63F0] TRAP #5 ()                            // Sys_Yield (0 args): yield 1 frame; resumes @ 0C63F2
 5.   [0C63F2] [0x408917] = [0x408916]               // Latch debug hitbox-draw request flag
 6.   [0C63F8] call 0x0C6446()                       // AABB: List 2 (player shots 0x41ABE0..) vs List 1 (enemies 0x41A988..)
                                                     //   Hit: A3+0x35=1, A2+0x35=0xFF, cross-link +0x36, sub +0x28 dmg from +0x24 hp
                                                     //   (gated by A3+0x34 bits 0/1); bit 6 = piercing (keep testing A3s)
 7.   [0C63FC] call 0x0C64FC()                       // Loop P1 (0x41B190) & P2 (0x41B21A, stride 0x8A):
                                                     //   Skip if +0x34!=0 or (+0x65 bit 2)!=0.
                                                     //   D1.w = pos_x+hb_x0, D2.w = pos_x+hb_x1, D3.w = pos_y+hb_y0, D4.w = pos_y+hb_y1.
                                                     //   Walk A3 backwards from A2=*(0x408906) down to D0=0x41ACE0 (List 3 hazards):
                                                     //     A4 = *(--A3); if bomb_dmg[p]>0 (0x41A980+4p) & !(A4+0x34 bit 5) &
                                                     //       (0x409D58 bit 0 || !(A4+0x34 bit 4)): mark A4+0x35=2, A4+0x36=A0, hp-=bomb_dmg.
                                                     //     If !(0x409D58 bit 0) && AABB overlap(player, A4):
                                                     //       A4+0x35=2, A4+0x36=A0, A0+0x36=A4; sub dmg (+0x28) from hp (+0x24) per A4+0x34 bits 1/0.
 8.   [0C6400] call 0x0C6624()                       // Latch FG scroll int words: [0x408918]=[0x402258], [0x40891A]=[0x40225C]
 9.   [0C6404] call 0x0C640A()                       // Snapshot list ends 0x4088FE..906->0x40890A..912; reset to 0x41A988/ABE0/ACE0;
                                                     //   clr.l (A0)+ x2 at 0x41A980 -> leaves A0 = 0x41A988
10.   [0C6408] goto LOOP                             // bra.b 0x0C63F0
```
- **TRAPs in `0xC63E8`–`0xC6A63`:** Only `0xC63F0: trap #0x5` (`Sys_Yield`, no stack args) (**CONFIRMED**).
- **Register state at `0xC63F0` yield (1P active, P2 skipped):** `D0.w=0xFFFF` (dbf end), `D1.w=P1.x+P1.hb_x0`, `D2.w=P1.x+P1.hb_x1`, `D3.w=P1.y+P1.hb_y0`, `D4.w=P1.y+P1.hb_y1`, `D5.w=0xFFFF`, `A0=0x41A988` (after `0xC643E`), `A1=0x41A988` (after `0xC6600` ×2), `A2=*(0x408906)` (`0x41ACF0` = 4 entries in List 3), `A3=0x41ACE0` (List 3 base), `A4` = last List 3 `EntityObj*` (`0x4143E0`, `obj_pool` group 5 slot 0) (**CONFIRMED**).

### B. Player Movement Object Handler (`FUN_000c6866` → `FUN_000c691a`, `0xC6866`–`0xC6A62`) — **CONFIRMED**
```text
 1. [0C6866] For A0 = 0x41B190 (P1), 0x41B21A (P2) (D0w=1..0): if *(s16*)(A0+0x4C) >= 0:
               call 0x0C691A(); call 0x0C8682(); call 0x0C689E()   // move/bounds, shield render, cockpit pal pulse
               [0C685E] *obj = 0x0C6866; return                    // re-arm object pool callback
 2. [0C691A] If (*(u8*)(A0+0x65) & 1) return                       // Movement locked flag
 3. [0C6924] push old_y = *(s16*)(A0+0x18)                         // Save Y for banking check
 4. [0C6928] If !([0x409D36] & 0x08):                              // Controls enabled
      [0C6932] nibble = (**(u8**)(A0+0x44)) & 0x0F                 // Active-low io_ch[2].cur (0x402224) / io_ch[3].cur (0x402228)
      [0C693C] angle = ((s16*)0x0C9462)[nibble]; *(s16*)(A0+0x20) = angle
      [0C694A] if (angle >= 0) call 0x0CBE12(A0, D0w=angle)        // pos_x += speed*cos[angle]; pos_y += speed*sin[angle]
 5. [0C6954] If ([0x409D36] & 0x10):                               // Turbulence/shake (set in 0x13DAD4)
      [0C695E..76] pos_x.l += (s32)Rng_Next()<<1; pos_y.l += (s32)Rng_Next()<<1
 6. [0C697A] If (*(s16*)(A0+0x50) <= 0):                           // Invulnerability timer expired
      [0C6982..9A] if (call 0x0C6632(D0w=pos_x+0x14, D1w=pos_y+0x0E) != 0): // Hit solid terrain bit?
        [0C699E] *(s32*)(A0+0x24) -= 0x10000                       // Subtract 1.0 HP
        [0C69A6] if (*(s16*)(A0+0x58) < 10) *(s16*)(A0+0x4E) = 0   // Weak shield: break shield
        [0C69B8] else { call 0x0CBE00(A0);                         // Undo scroll delta (0x40C224/28)
                        if (angle >= 0) { speed=9; call 0x0CBE12(angle-0x80); speed=12; } } // Push back opposite dir
 7. [0C69DC..A1A] Clamp *(s16*)(A0+0x14) to [-8, 0x118] (-8..280); clamp *(s16*)(A0+0x18) to [0x0E, 0xD2] (14..210)
 8. [0C6A1C..A62] Bank tilt *(s32*)(A0+0x3A) (16.16, clamped [-2,+2]): +0x2000 if moving up (or <0 idle), -0x2000 if down (or >0 idle)
```
- **Input bits (`*(A0+0x44)` → `io_ch[2]` `0x402224` P1 / `io_ch[3]` `0x402228` P2, active-low, ROM `0xC9462` + `0xC9262/68`):** bit 0 = **Up** (`0xE`→angle `0xC0`), bit 1 = **Down** (`0xD`→`0x40`), bit 2 = **Left** (`0xB`→`0x80`), bit 3 = **Right** (`0x7`→`0x00`); diagonals `0x6/5/A/9`→`0xE0/20/A0/60`; neutral `0xF`/invalid→`-1` (**CONFIRMED**). Fire/Bomb (`io_ch[n]` bits 4 & 5, confirmed in `0x13D7A0` via `pbVar2[1] & 0x10`/`0x20`) are **not** read in `0xC63E8`–`0xC6A63` (**CONFIRMED**).
- **Speed & bounds:** `speed` (`+0x42`) = `0x000C` (12, `0xC69D6`; recoil `0x0009`, `0xC69CA`). With 16.16 `pos_x`/`pos_y` and `cos`/`sin` unit `16384` (`0x27EC0`/`0x27E40`), `12 * 16384 = 0x30000` = **3.000 px/frame** cardinal, `2.121 px/frame` diagonal (**CONFIRMED**). Bounds: `X ∈ [-8, 280]` (`0xFFF8..0x0118`, `0xC69DC`–`0xC69F6`), `Y ∈ [14, 210]` (`0x000E..0x00D2`, `0xC69FC`–`0xC6A16`) (**CONFIRMED**).
- **Shot spawning & death/respawn:** **Absent** from `0xC63E8`–`0xC6A63` (**CONFIRMED**). Shots are collided in `0xC6446`; HP depletion happens in `0xC65F4` (hazard) and `0xC699E` (terrain); respawn lives outside this range (e.g. `0xC6D0E` resets `hp=0x10000`, `invuln=0xD2`, spawn X/Y from `0xC926E`).

---

## 2. Subroutines in Range & Callees

| Addr | Proposed Name | Purpose | Status |
|---|---|---|---|
| `0x0C63E8` | `Task_Player1Ship` (`Task_CollisionSync`) | Slot 3 main loop: latches debug flag, runs shot/player collisions, syncs terrain scroll, resets lists, `TRAP #5` | **CONFIRMED** |
| `0x0C640A` | `Coll_ResetLists` | Snapshots list ends (`8FE..906→90A..912`), resets cursors (`41A988/ABE0/ACE0`), zeroes `41A980[2]` | **CONFIRMED** |
| `0x0C6446` | `Coll_PlayerShotsVsEnemies` | Nested AABB test: List 2 (`0x41ABE0` shots) vs List 1 (`0x41A988` shootables); writes hit flags & HP damage | **CONFIRMED** |
| `0x0C64FC` | `Coll_PlayersVsHazards` | Loops P1/P2 (`0x41B190`, stride `0x8A`) vs List 3 (`0x41ACE0`); applies area/bomb dmg (`0x41A980`) & AABB hits | **CONFIRMED** |
| `0x0C660A` | `Coll_AddAreaDamage` | Adds `D0.l` to `0x41A980[player_id*4]` where `player_id = *(u16*)(D1+0x22)` (called by `0xC6D0E`) | **CONFIRMED** |
| `0x0C6624` | `Terrain_SyncScroll` | Copies FG scroll words `0x402258`/`0x40225C` → `0x408918`/`0x40891A` | **CONFIRMED** |
| `0x0C6632` | `Terrain_TestPoint` | Tests 1 bit in 64×640 terrain bitmap `0x40891C` at `((X+scrX)>>3, (Y+scrY)>>3)`; `0` if `0x409D1C!=0` | **CONFIRMED** |
| `0x0C666A` | `Terrain_OrMask16` | ORs `count` 16-bit rows from `(A1)+` into terrain bitmap `0x40891C` via `bfextu`/`bfins` | **CONFIRMED** |
| `0x0C66AE` | `Terrain_XorMask16` | Undisassembled raw bytes (`0xC66AE`–`0xC66F3`): identical to `0xC666A` with `eor.w D3w,D2w` | **CONFIRMED** |
| `0x0C66F4` | `Terrain_AndMask16` | Undisassembled prologue (`0xC66F4`–`0xC66FF`) into `LAB_000c6700`: `and.w (A1)+,D2w` (clears terrain) | **CONFIRMED** |
| `0x0C6738` | `Terrain_LoadStageMap` | Decompresses stage terrain RLE from `ROM 0x16AD1E[stage*4]` into `0x40891C` (`0x1400` B) via `0x16A2F6` | **CONFIRMED** |
| `0x0C676E` | `Debug_DrawHitboxes` | Called by `Task_Slot15Daemon` (`0x38AC`); if `0x408917!=0`, draws 4 corner sprites for List 3, List 2, P1/P2 | **CONFIRMED** |
| `0x0C67EA` | `Debug_DrawEntityBox` | Emits 4 corner sprites (`0xC6836`) for entity `A0`'s AABB (`+0x14/+0x18` + `+0x2C..32`), toggling flip bits 24–25 | **CONFIRMED** |
| `0x0C6836` | `Sprite_AppendRaw16` | Writes 16-byte sprite entry `(D6, D0&FFF, D1&FFF, D5, 0)` to `*(0x407360)` and advances cursor `+0x10` | **CONFIRMED** |
| `0x0C685A` | `Obj_PlayerTickInit` | Clears `(6,A0)` and sets object callback `*(A0)=0x0C6866` (spawned at `0xC6A88`) | **CONFIRMED** |
| `0x0C6866` | `Obj_PlayersTick` | Loops P1 & P2 (`0x41B190`, stride `0x8A`): if `+0x4C>=0`, calls `0xC691A`, `0xC8682`, `0xC689E`; re-arms `*A0` | **CONFIRMED** |
| `0x0C689E` | `Player_PulsePalette` | 20-frame cycle (`+0x6`): fades pal `0x4078+(+0xC)*0x40` black↔`0xC6912[id]` (`0x3BCBFB`/`0x8BD303`) via `0x1866`/`0x1890` | **CONFIRMED** |
| `0x0C691A` | `Player_MoveAndBounds` | Reads joystick, moves ship, applies shake, checks terrain hit/recoil, clamps screen bounds, updates bank tilt | **CONFIRMED** |
| `0x001866` | `Pal_FadeFromBlack` | External: schedules 10-frame fade from `0x190E` (`0x000000`) to `arg0` color via `0x1792`→`0x17D6` | **CONFIRMED** |
| `0x001890` | `Pal_FadeToBlack` | External: schedules 10-frame fade from `arg0` color to `0x190E` (`0x000000`) via `0x1792`→`0x17D6` | **CONFIRMED** |
| `0x001D8A` | `Rng_Next` | External: 32-bit LCG `seed=seed*7+5`, returns `D0.w = seed>>16` | **CONFIRMED** |
| `0x0C8682` | `Player_UpdateShield` | External: ticks shield timer (`+0x5E`) / hits (`+0x4E`) and draws shield sprite via `0xC8460`→`0x1282` | **CONFIRMED** |
| `0x0CBE00` | `Obj_SubScrollVel` | External: subtracts `0x40C224`/`0x40C228` from `A0+0x14`/`A0+0x18` (**GUESS** stage scroll velocity) | **CONFIRMED** |
| `0x0CBE12` | `Obj_MoveByAngle` | External: `pos_x += speed*cos[D0w]` (`0x27EC0`), `pos_y += speed*sin[D0w]` (`0x27E40`), `speed=*(s16*)(A0+0x42)` | **CONFIRMED** |
| `0x16A2F6` | `Rle16_Decompress` | External: fills `(A0)+` with `(word, count)` pairs from `(A1)+` until `-1` sentinel | **CONFIRMED** |

---

## 3. Structures Used

### Pointers Observed at `0xC63F0` Yield (**CONFIRMED**)
- **`A0 = A1 = 0x41A988`**: Immediately past `player_bomb_dmg[2]` (`0x41A980..0x41A987`) = base of **Collision List 1** (`EntityObj*[]`, shootable enemies, `0x41A988..0x41ABDF`, max 150 ptrs).
- **`A2 = 0x41ACF0`**: End cursor `*(0x408906)` of **Collision List 3** (4 active hazard pointers at yield).
- **`A3 = 0x41ACE0`**: Base of **Collision List 3** (`EntityObj*[]`, player-damaging enemies/bullets).
- **`A4 = 0x4143E0`**: Last popped hazard `EntityObj*` in List 3 (`obj_pool` `0x410060 + 5*32*0x6C`, group 5 slot 0).

### Common `EntityObj` Header (`obj_pool`, size `0x6C`) & `PlayerState` (`0x41B190` P1 / `0x41B21A` P2, size `0x8A`)
Both share offsets `0x00..0x39` so `SUB_000c6446`/`000c64fc`/`000c67ea` treat them uniformly (**CONFIRMED**):

| Offset | Size | Proposed Name | Struct | Status & Evidence |
|---|---|---|---|---|
| `+0x00` | 4 B | `tick_fn` | Obj | **CONFIRMED** callback ptr written at `0xC685E` (`#0xC6866`) |
| `+0x06` | 2 B | `pal_pulse_cnt` | Player | **CONFIRMED** 0..19 counter in `0xC68B6`–`0xC690C` (also cleared on obj at `0xC685A`) |
| `+0x08` | 4 B | `owner_ptr` | Obj | **CONFIRMED** shot→player ptr read in `0xD0E04` when `hit_type==1` |
| `+0x0C` | 1 B | `pal_bank` | Player | **CONFIRMED** shifted `<<6` + `0x4078` for palette address at `0xC68AC`–`0xC68B2` |
| `+0x14` | 4 B | `pos_x` (16.16) | Both | **CONFIRMED** high word = screen X (`0xC6466`, `0xC6530`, `0xC69DC`); long add in `0xC6968`/`0xCBE12` |
| `+0x18` | 4 B | `pos_y` (16.16) | Both | **CONFIRMED** high word = screen Y (`0xC6476`, `0xC6540`, `0xC69FC`); long add in `0xC6976`/`0xCBE12` |
| `+0x20` | 2 B | `move_angle` | Player | **CONFIRMED** 0..255 angle (or `-1` idle) stored at `0xC6946`, read at `0xC69BE` |
| `+0x22` | 2 B | `player_id` | Player | **CONFIRMED** `0` (P1) or `1` (P2); indexes `0x41A980` (`0xC6610`) & `0xC6912` (`0xC689E`) |
| `+0x24` | 4 B | `hp` (16.16) | Both | **CONFIRMED** decremented by `+0x28` (`0xC64D8/EA`, `0xC658A/E2/F4`) & `0x10000` (`0xC699E`) |
| `+0x28` | 4 B | `contact_dmg` | Both | **CONFIRMED** damage dealt on collision (`0xC64D4/E6`, `0xC65DE/F0`, init `0x10000` in `0xC6D0E`) |
| `+0x2C..32` | 4×2 B | `hb_x0,hb_x1,hb_y0,hb_y1` | Both | **CONFIRMED** signed AABB offsets relative to `(pos_x, pos_y)` (`0xC645E..7A`, `0xC6528..44`, `0xC67EA`) |
| `+0x34` | 1 B | `coll_flags` | Both | **CONFIRMED** Player: nonzero disables hit (`0xC6516`). Obj: bit 0=no dmg to other, bit 1=invuln, bit 4=bomb-immune, bit 5=skip area dmg, bit 6=piercing (`0xC64CA..F4`, `0xC6552..E6`) |
| `+0x35` | 1 B | `hit_type` | Both | **CONFIRMED** `1`=shot hit (`0xC64B8`), `0xFF`=shot consumed (`0xC64BE`), `2`=player/bomb hit (`0xC6576/C6`; checked in `0xD0E04`) |
| `+0x36` | 4 B | `hit_by_ptr` | Both | **CONFIRMED** pointer to colliding entity/player (`0xC64C2/C6`, `0xC657C/CC/D0`; used in `0xD0E04`) |
| `+0x3A` | 4 B | `bank_tilt` (16.16) | Player | **CONFIRMED** vertical banking anim `[-2.0, +2.0]` stepped by `±0x2000` (`0xC6A2A`–`0xC6A5A`) |
| `+0x42` | 2 B | `move_speed` | Player | **CONFIRMED** `12` normal (`0xC69D6`), `9` terrain recoil (`0xC69CA`); multiplied in `0xCBE12` |
| `+0x44` | 4 B | `io_ch_ptr` | Player | **CONFIRMED** points to `io_ch[2]` (`0x402224`) / `io_ch[3]` (`0x402228`) (`0xC6932`, `0x13D7A0`) |
| `+0x48` | 4 B | `score` | Player | **CONFIRMED** `0x41B190+0x48 = 0x41B1D8` (`score_p1`, written in `0xD0E04`) |
| `+0x4C` | 2 B | `lives_or_state` | Player | **CONFIRMED** `< 0` skips player tick (`0xC6874`, `0x1609EC`); **GUESS** remaining lives/active |
| `+0x4E` | 2 B | `shield_hp` | Player | **CONFIRMED** cleared on terrain hit if `+0x58<10` (`0xC69B0`); decremented & drawn in `0xC8682` |
| `+0x50` | 2 B | `invuln_timer` | Player | **CONFIRMED** `> 0` skips terrain hit (`0xC697A`); init `0xD2` (210f) on respawn (`0xC6D0E`) |
| `+0x58` | 2 B | `arm_level` | Player | **CONFIRMED** `< 10` breaks shield vs `>= 10` bounces off terrain (`0xC69A6`); **GUESS** Arm/shield upgrade level |
| `+0x65` | 1 B | `player_flags` | Player | **CONFIRMED** bit 0=lock move (`0xC691A`), bit 2=disable coll/shield (`0xC651E`, `0xC8682`) |

---

## 4. Work RAM Globals Touched (`A5 = 0x408000`)

| Abs Addr | A5 Offset | Size | Proposed Name | Status & Evidence |
|---|---|---|---|---|
| `0x402224`/`28` | `-0x5DDC`/`D8` | 4 B ×2 | `io_ch[2]`/`io_ch[3]` | **CONFIRMED** P1/P2 active-low inputs read indirectly via `(0x44,A0)` at `0xC6932` |
| `0x402258`/`5C` | `-0x5DA8`/`A4` | 2 B ×2 | `fg_scroll_x`/`y` | **CONFIRMED** high words of PF0 scroll shadow (`0x38AC`) copied to `0x408918/1A` (`0xC6624`) |
| `0x407360` | `-0x0CA0` | 4 B | `sprite_cur_ptr` | **CONFIRMED** `SPRITE_RAM` write ptr (`0x117E`, `0x1282`), appended in `0xC6836`–`0xC6854` |
| `0x4088FE` | `+0x08FE` | 4 B | `coll_list1_end` | **CONFIRMED** shootable enemy ptr cursor (base `0x41A988`), reset `0xC641C`, read `0xC647E` |
| `0x408902` | `+0x0902` | 4 B | `coll_list2_end` | **CONFIRMED** player shot ptr cursor (base `0x41ABE0`), reset `0xC6424`, read `0xC6446`/`C67B0` |
| `0x408906` | `+0x0906` | 4 B | `coll_list3_end` | **CONFIRMED** hazard ptr cursor (base `0x41ACE0`), reset `0xC642C`, read `0xC650E`/`C6798` |
| `0x40890A..12` | `+0x090A..12` | 3×4 B | `coll_list1..3_prev` | **CONFIRMED** previous-frame snapshots of `0x4088FE..906` (`0xC640A`–`0xC6416`) |
| `0x408916`/`17` | `+0x0916`/`17` | 1 B ×2 | `dbg_hb_req`/`active` | **CONFIRMED** hitbox debug overlay flag latched `0xC63F2`, tested & cleared `0xC6772`/`C67E0` |
| `0x408918`/`1A` | `+0x0918`/`1A` | 2 B ×2 | `terrain_scr_x`/`y` | **CONFIRMED** latched FG scroll X/Y added before `>>3` in `0xC6640`/`C666A`/`C66AE`/`C66F4` |
| `0x40891C` | `+0x091C` | `0x1400` B | `terrain_bitmap` | **CONFIRMED** 64×640 1-bpp 8×8-tile solid map (`bftst`/`bfextu`/`bfins`, loaded `0xC674A`) |
| `0x409D1C` | `+0x1D1C` | 1 B | `terrain_coll_off` | **CONFIRMED** cleared `0xC63E8`; nonzero forces `Terrain_TestPoint`=`0` (`0xC6632`; set `0x13DAD4`) |
| `0x409D36` | `+0x1D36` | 1 B | `player_ctrl_flags` | **CONFIRMED** bit 3=disable joystick (`0xC6928`), bit 4=RNG shake (`0xC6954`, `0x13DAD4`) |
| `0x409D58` | `+0x1D58` | 1 B | `bomb_active_flags` | **CONFIRMED** bit 0=bypass bomb-immunity & skip player AABB hit (`0xC6562`/`C658E`); **GUESS** black-hole bomb |
| `0x40B2DC` | `+0x32DC` | 2 B | `stage_index` | **CONFIRMED** index into `0x16AD1E` terrain RLE table (`0xC6742`) & stage tables (`0x13CAAE`) |
| `0x40C224`/`28` | `+0x4224`/`28` | 4 B ×2 | `stage_scroll_vel_xy` | **CONFIRMED** subtracted from player pos in `0xCBE00` on terrain recoil; **GUESS** scroll delta |
| `0x41A980` | abs | 2×4 B | `player_bomb_dmg[2]` | **CONFIRMED** per-player area dmg added by `0xC660A`, applied `0xC655C`, cleared `0xC6434` |
| `0x41A988`/`ABE0`/`ACE0` | abs | ptr arrays | `coll_list1`/`2`/`3` | **CONFIRMED** entity ptr lists: shootables (`0x41A988`), shots (`0x41ABE0`), hazards (`0x41ACE0`) |
| `0x41B190`/`B21A` | abs | 2×`0x8A` B | `players[2]` | **CONFIRMED** P1 (`0x41B190`, `score_p1` at `+0x48`=`0x41B1D8`) & P2 (`0x41B21A`) state structs |
