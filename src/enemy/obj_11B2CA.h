#ifndef TAITOF3_OBJ_11B2CA_H
#define TAITOF3_OBJ_11B2CA_H
#include "../common/f3mem.h"
/* The shared tracker helpers (ObjHelper_11B262/11B288/11B2CA) are emitted
 * once in obj_11B262.c; this header pulls in their declarations. */
#include "obj_11B262.h"

/* Object tick routine (A0 = obj). Returns 0, or -1 when it reaches a ROM
 * path not decompiled yet (no side effects of that path are applied). */
int Obj_11B2CA_Tick(const f3mem_t *m, uint32_t obj); /* ROM 0x11B2CA */
#endif
