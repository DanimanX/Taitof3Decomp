#ifndef TAITOF3_OBJ_11B262_H
#define TAITOF3_OBJ_11B262_H
#include "../common/f3mem.h"
/* Object tick routine (A0 = obj). Returns 0, or -1 when it reaches a ROM
 * path not decompiled yet (no side effects of that path are applied). */
int Obj_11B262_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x11B262 */

/* Shared tracker helpers, called from ~40 object routines. The bodies live in
 * obj_11B262.c only; callers include this header instead of repeating them
 * (obj_11B288.h and obj_11B2CA.h include it too). obj is an object record
 * (OBJ_SIZE bytes, see obj_pool.h). */

/* ROM 0x11B262: copy the leader position into the object, move it along
 * +0x20, then publish the moved position back to the leader. */
void ObjHelper_11B262(const f3mem_t *m, uint32_t obj);

/* ROM 0x11B288: turn +0x20 toward the leader position by at most 2/frame. */
void ObjHelper_11B288(const f3mem_t *m, uint32_t obj);

/* ROM 0x11B2CA: ease the angle at +0x56 toward the target byte at +0x54 and
 * mirror it into +0x42 (asr.w #8). */
void ObjHelper_11B2CA(const f3mem_t *m, uint32_t obj);
#endif
