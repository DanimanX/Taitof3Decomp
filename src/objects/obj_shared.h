#ifndef TAITOF3_OBJECTS_OBJ_SHARED_H
#define TAITOF3_OBJECTS_OBJ_SHARED_H
/* Primitives every object routine reaches through a jsr and that were, until
 * now, re-implemented as a `static` helper in each of the ~60-90 files that
 * need them. One verified copy lives in obj_shared.c; the rest call it.
 *
 * Sharing them is what unblocks the planner's dependency gate: a routine whose
 * `; calls:` line names 0x0CBB2C / 0x0CBE34 / 0x0D0E30 / 0x0D0E3A used to wait
 * for its own private copy and never became "ready". */
#include "../common/f3mem.h"

/* ROM 0x0CBB2C: explosion / death animation. Orients the sprite along the
 * {dx,dy}.w record at obj+0x10, clamps it to the blast radius A5+0x1D5C and
 * either bails to tick 0x0CBC88 or continues to 0x0CBC78 after drawing. */
void Obj_ExplodeTick(const f3mem_t *m, uint32_t obj);

/* ROM 0x0CBE34: blinking colour override on obj+0x1D, toggling obj+0x35. */
void Obj_BlinkColour(const f3mem_t *m, uint32_t obj);

/* ROM 0x0D0E30: add D0 to the 32-bit timer at (A5+0x1D50)->+0x48. */
void Obj_RelayTimer(const f3mem_t *m, uint32_t d0);

/* ROM 0x0D0E3A: 4-way interleaved LCG (x' = 5x + 1) on A5+0x1D60; returns the
 * high byte of the result (D0.w >> 8). */
uint16_t Obj_Next4Way(const f3mem_t *m);

/* ROM 0x0CF1D8: install the object script at A1 and request a restart. */
void Obj_StartScript(const f3mem_t *m, uint32_t obj, uint32_t a1);

/* ROM 0x006422: integer atan2 (D0.w = dy, D1.w = dx) -> angle in D0.b, via the
 * 0x026E40 arctan table; the quadrant flags live in the 0x006492 table. */
uint16_t Obj_Atan2(const f3mem_t *m, uint16_t d0, uint16_t d1);

/* ROM 0x00649A: integer hypot (D0.w, D1.w) -> distance in a 4.12 fixed point
 * scale determined by the magnitude, via the 0x028340 log table. */
uint16_t Obj_Distance(const f3mem_t *m, uint16_t d0_in, uint16_t d1_in);

#endif
