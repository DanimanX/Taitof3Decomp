#ifndef TAITOF3_OBJECTS_OBJ_SHARED_EXT_H
#define TAITOF3_OBJECTS_OBJ_SHARED_EXT_H
/* Companion of obj_shared.h: the block-level primitives the object ticks reach
 * through a jsr, written once here instead of being copied into every routine
 * that calls them. Bodies follow the ROM listings (work/disasm/bundle_*.s). */
#include "../common/f3mem.h"

/* ROM 0x0CEC4C: seed an object from the parameter record at A1 -- the +0x1C/
 * +0x28 head, the sprite list at +0x2C, the accel/cap longs at +0x30, then the
 * 0x14-word state block 0x0CECE4[class==5 ? 0 : +0x5] into the object's +0xC
 * pointer. Returns that state block pointer. */
uint32_t Obj_SeedFromParams(const f3mem_t *m, uint32_t obj, uint32_t a1);

/* ROM 0x0CEE2C: colour override from the object's class through 0xCEE40[]. */
void Obj_ClassColour(const f3mem_t *m, uint32_t obj);

/* ROM 0x0D0678: install the behaviour record at A1 ({dur.w, timer.w, spr.l,
 * ptr.l}) into +0x48/+0x4C/+0x1C/+0x10/+0x68. */
void Obj_StartBehaviour(const f3mem_t *m, uint32_t obj, uint32_t a1);

/* ROM 0x1794A4: random value in [-(n-1)..n-1] from the LCG at A5+0x4B3C
 * (x' = 5x - 1, halves swapped back), n = 0 returns 0. */
uint16_t Obj_RandRange(const f3mem_t *m, uint16_t n);

/* ROM 0x0D0E52: 256-step direction of the point (x, y) -- quadrant offset from
 * 0xD0EB4 plus the BFFFO-scaled 0xD0EBC byte table. */
uint16_t Obj_Atan256(const f3mem_t *m, int16_t x, int16_t y);

/* ROM 0x006422: byte angle (0x40 = +y) of the point (x, y) from the 0x026E40
 * table; the 0x006492 offsets carry the quadrant. */
uint16_t Obj_AtanByte(const f3mem_t *m, int16_t x, int16_t y);

/* ROM 0x00649A: integer hypot of (x, y) on the 0x028340 log table, returned in
 * the scaled form the 0xD0E52-family callers expect; *shift gets D1.w. */
uint16_t Obj_DistScaled(const f3mem_t *m, int16_t x, int16_t y, uint16_t *shift);

/* ROM 0x179434: map absolute position (+0x14/+0x18) back to screen-relative
 * coordinates through the velocity/scale word in +0x58. */
void Obj_BossMapScreenPos(const f3mem_t *m, uint32_t obj);

/* ROM 0x17AF9A: byte angle from (x, y) toward player object p (with +0x18/+0x10
 * centre offset) via 0x006422; returns (uint8_t)p if p <= 0. */
uint8_t Obj_AimAtPlayer17AF9A(const f3mem_t *m, uint32_t p, uint16_t x, uint16_t y);

/* ROM 0x17B3AE: queue upload of the 8 boss palettes (alternate set when
 * A5+0x32DC == 1) via 0x001660. */
void Boss_UploadPalettes(const f3mem_t *m);

/* ROM 0x1847E8: reset the boss globals at A5+0x4B02.. and object flags. */
void Boss_ResetGlobals(const f3mem_t *m, uint32_t obj);

/* ROM 0x184888 / 0x1848D6: seed boss/enemy stats & hitboxes from record A1. */
void Obj_184888(const f3mem_t *m, uint32_t obj, uint32_t a1);
void Obj_1848D6(const f3mem_t *m, uint32_t obj, uint32_t a1);

/* ROM 0x13F238: remap sound command (+0x53 when A5+0x3306 != 0 and 1 <= cmd <= 0x40). */
int16_t Sound_PreCmdHook(const f3mem_t *m, int16_t cmd);

/* ROM 0x11B2F6: reset enemy fields. */
void Obj_11B2F6(const f3mem_t *m, uint32_t obj);

/* ROM 0x113652: init child object from +0x0C packed offset/angle and +0x08 parent. */
void Obj_113652(const f3mem_t *m, uint32_t obj);

/* ROM 0x17A7FA: clear common boss/enemy object fields. */
void Obj_17A7FA(const f3mem_t *m, uint32_t obj);

/* ROM 0x18495E: scale boss HP (+0x24 high word) by difficulty nibble A5+0x87F. */
void Obj_18495E(const f3mem_t *m, uint32_t obj);

/* ROM 0x11AE2C: clear bg-scroll/gfx state and free object. */
void Obj_11AE2C(const f3mem_t *m, uint32_t obj);

/* ROM 0x0CEDE4: seed enemy HP/dmg/hitboxes from parameter record A1+0x14. */
void Obj_0CEDE4(const f3mem_t *m, uint32_t obj, uint32_t a1);

/* ROM 0x0CFA7C: index into offset table at 0x0CFA96 and load sprite/ptr from A1. */
void Obj_0CFA7C(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d0);

/* ROM 0x0CBDDE: resolve score receiver from +0x36 (or (+0x36)->+0x08 when +0x35 == 1). */
uint32_t Obj_0CBDDE(const f3mem_t *m, uint32_t obj);

/* ROM 0x13E4D0: pack sprite center offset (w*8, h*8) into high 16 bits of d0. */
uint32_t Obj_13E4D0(const f3mem_t *m, uint32_t obj, uint32_t d0);

/* ROM 0x0CC3C6: angle from obj (A0) hitbox center to target (A1) hitbox center. */
uint16_t Obj_0CC3C6(const f3mem_t *m, uint32_t obj, uint32_t target);

/* ROM 0x11C32C: update flash/palette byte +0x1D from +0x35/+0x4F. */
void Obj_11C32C(const f3mem_t *m, uint32_t obj);

/* ROM 0x11B06E: push to collision lists if active & hitbox non-zero, then Obj_11C32C. */
void Obj_11B06E(const f3mem_t *m, uint32_t obj);

/* ROM 0x11B09A: compute screen pos from +0x50/+0x54/+0x30D2 and draw if visible. */
void Obj_11B09A(const f3mem_t *m, uint32_t obj);

/* ROM 0x0D0DC2: push obj to list1 (A5+0x8FE) and list3 (A5+0x906). */
void Obj_0D0DC2(const f3mem_t *m, uint32_t obj);

/* ROM 0x0CEF0E: off-screen/bomb handling; parks at 0x000CEF44 when off-screen. */
void Obj_0CEF0E(const f3mem_t *m, uint32_t obj);

/* ROM 0x0CF898: 3D-orbit offset step from descriptor at +0x0C -> (d0, d1). */
int Obj_0CF898(const f3mem_t *m, uint32_t obj, uint32_t *out_d0, uint32_t *out_d1);

/* ROM 0x0D0DE4: push obj to list1 (A5+0x8FE) and list3 (A5+0x906) if bit7 of +0x07 clear. */
void Obj_0D0DE4(const f3mem_t *m, uint32_t obj);

/* ROM 0x0D0E04: add d0 to parent/grandparent +0x48 timer based on +0x35 (1 or 2). */
void Obj_0D0E04(const f3mem_t *m, uint32_t obj, uint32_t d0);

/* ROM 0x0CEE48: spawn 0x117C28 score/fx if +0x4E != 0 and not bomb-clear, then Obj_StartDying. */
void Obj_0CEE48(const f3mem_t *m, uint32_t obj);

/* ROM 0x0D825E: 3D perspective screen pos from +0x54/+0x58/+0x60 -> +0x14/+0x18/+0x1E/+0x1F. */
void Obj_0D825E(const f3mem_t *m, uint32_t obj);

/* ROM 0x0D8318: draw scaled sub-part sprite at (d0, d1) offset from obj. */
void Obj_0D8318(const f3mem_t *m, uint32_t obj, uint32_t a2, int16_t d0, int16_t d1);

/* ROM 0x0D8294: draw multi-part scaled boss/enemy sprite from table 0x0D8446[+0x5C]. */
void Obj_0D8294(const f3mem_t *m, uint32_t obj);

/* ROM 0x0D8378: step +0x42/+0x22/+0x21 and integrate +0x54/+0x58 via 0x027E40 sin/cos. */
void Obj_0D8378(const f3mem_t *m, uint32_t obj);

/* ROM 0x0D70AE: Terrain_TestPoint wrapper applying +0x64 flip bits and +0x14/+0x18 pos. */
uint16_t Obj_0D70AE(const f3mem_t *m, uint32_t obj, int16_t d0, int16_t d1);

/* ROM 0x11B39A: init spawner at X=0x140, Y=0x2D+(rng&0x7F), spawn 0x11B3D2. */
void Obj_11B39A(const f3mem_t *m, uint32_t obj);

/* ROM 0x16AB26: write (d0, d1) longs to (a0)+. */
void Obj_16AB26(const f3mem_t *m, uint32_t a0, uint32_t d0, uint32_t d1);

/* ROM 0x116BE6 / 0x117058 / 0x1170EE / 0x117102 / 0x117154 / 0x117186 / 0x116F12 / 0x116FB0:
 * multi-segment 3D chain/boss helpers. */
void Obj_116BE6(const f3mem_t *m, uint32_t obj);
void Obj_1170EE(const f3mem_t *m, uint32_t obj, uint32_t a1);
void Obj_117058(const f3mem_t *m, uint32_t obj, uint32_t a1);
void Obj_117102(const f3mem_t *m, uint32_t a1);
void Obj_117154(const f3mem_t *m, uint32_t obj, uint16_t d0);
void Obj_117186(const f3mem_t *m, uint32_t obj, uint16_t d0, uint16_t d1);
void Obj_116F12(const f3mem_t *m, uint32_t a1);
void Obj_116FB0(const f3mem_t *m, uint32_t a1);

#endif
