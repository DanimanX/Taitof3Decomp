#ifndef TAITOF3_OBJ_SCRIPT10_H
#define TAITOF3_OBJ_SCRIPT10_H
#include "../common/f3mem.h"

/* The 10-byte movement script walked by 0x178982.
 *
 * Script pointer lives at obj+0x44. Each entry is
 *     { fn.l, frames.w, arg0.l }        (10 bytes)
 * fn == 0 or fn < 0 marks a loop point: the repeat count at obj+0x4A is armed
 * from frames.w and the walk jumps to arg0 (the target must be a positive
 * address; a non-positive one falls through to the next entry).
 *
 * Commands are dispatched by absolute address; only the two translated here
 * return 0. An unknown command makes Obj_Script10 return -1 so the caller can
 * report an unobservable path instead of guessing.
 *
 * Bodies lifted from src/enemy/obj_17C05E.c (same ROM addresses), where the
 * interpreter is already exercised against a passing capture. */

/* ROM 0x178982. Returns 0, or -1 when the script reaches an untranslated
 * command (no side effect of that step is applied). */
int Obj_Script10(const f3mem_t *m, uint32_t obj);

/* ROM 0x178B20: +0x42 := the current entry's arg0.w (a bare heading set). */
void Obj_Script10_SetHeading(const f3mem_t *m, uint32_t obj);

/* ROM 0x178E4A: steer toward the object at +0x4C using the entry's arg words
 * as turn rate and speed. */
void Obj_Script10_SteerTo(const f3mem_t *m, uint32_t obj);

int Obj_Script12(const f3mem_t *m, uint32_t obj); /* ROM 0x178A2C: 12-byte-record walk */
#endif
