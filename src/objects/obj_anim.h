#ifndef TAITOF3_OBJECTS_OBJ_ANIM_H
#define TAITOF3_OBJECTS_OBJ_ANIM_H
#include "../common/f3mem.h"
/* The 0x179A5C frame-animation engine.
 *
 * Twelve entries live in ROM at 0x179A5C, each {init.l, tick.l}:
 *
 *   idx  init     tick            idx  init     tick
 *    0   179550   179664           6   179632   179946
 *    1   1795F6   179880           7   179646   1799B2
 *    2   17960A   1798BA           8   17955E   1796B2
 *    3   179572   179700           9   179600   179898
 *    4   17961E   1798F8          10   179614   1798D4
 *    5   1795A4   17979C          11   179588   17974E
 *
 * An object dispatch is a `movea.l (0x0,A1,D0w*8),A1; jsr (A1)` with A1 =
 * 0x179A5E (the tick half), D0w = the index in obj+0x40. `init` and `tick`
 * arrive with the same A0, so a caller that only has the index can use one of
 * the two entry points below.
 *
 * Ticks are modelled for index 0 (the frame-list walk 0x179664, the one the
 * blocked routines reach) and index 9 (the 16-direction record 0x179898).
 * Any other index returns -1 before touching the object, which is the contract
 * every routine in this tree uses for a ROM path that is not translated yet. */

/* Tick-half dispatch: (0x179A5E)[idx] with A0 = obj. 0 on success, -1 for an
 * index without a translation. */
int Obj_AnimTick(const f3mem_t *m, uint32_t obj, uint16_t idx);

/* Init-half dispatch: (0x179A5C)[idx] with A0 = obj. 0 on success, -1 for an
 * index without a translation. */
int Obj_AnimInit(const f3mem_t *m, uint32_t obj, uint16_t idx);

/* ROM 0x179B06: pick one of two 0x179A5C frame lists with Obj_RandRange(1),
 * latch (+0x40 = index, +0x3A = record body) and run that list's INIT half.
 * Returns 0, or -1 through Obj_AnimInit for a list body not translated yet. */
int Obj_AnimList179B06(const f3mem_t *m, uint32_t obj);

/* ROM 0x179664: index-0 tick. Walks the frame list at +0x3A: a zero duration
 * advances to the next record (0x179550 reload), a positive one counts down in
 * +0x3E, a negative one ends the walk. */
void Obj_AnimTick0(const f3mem_t *m, uint32_t obj);

#endif
