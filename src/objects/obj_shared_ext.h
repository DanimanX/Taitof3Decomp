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

#endif
