#ifndef TAITOF3_OBJ_COMMON_H
#define TAITOF3_OBJ_COMMON_H
#include "../common/f3mem.h"

void Obj_StartDying(const f3mem_t *m, uint32_t obj);      /* ROM 0x0CEE74: fn = 0xCEE7C (frees after 3 frames) */
void Obj_FlagOffscreen(const f3mem_t *m, uint32_t obj);   /* ROM 0x0CF85A: if flags7 bit4, set bit3 when +0x3A/+0x3E leave [-64,383]x[-64,295] */
void Coll_AddHazard(const f3mem_t *m, uint32_t obj);      /* ROM 0x0D0DCC: append to hazard list (A5+0x906) */
void Obj_Animate(const f3mem_t *m, uint32_t obj, uint32_t anim_tbl); /* ROM 0x0CBD52: {dur.w, sprdef.l} frames, idx +0x3A, timer +0x3E */
void Obj_AnimFrameStep(const f3mem_t *m, uint32_t obj, uint32_t anim_tbl); /* ROM 0x0CBD24: one {dur.w, sprdef.l} step, idx +0x3A, timer +0x3E; returns nothing (D0 is incidental) */
void Obj_ResetFields(const f3mem_t *m, uint32_t obj); /* ROM 0x0CBD86: clear +0x1C/+0x1E/+0x34/+0x3A/+0x3E (words) and +0x24/+0x28 (longs) */
void Obj_StepScript(const f3mem_t *m, uint32_t obj);      /* ROM 0x0D0690: one 6-byte script step (timer +0x4C, ptr +0x48) */

int Obj_Dying1_Tick(const f3mem_t *m, uint32_t obj);  /* ROM 0x0CEE7C: fn = 0xCEE84 */
int Obj_Dying2_Tick(const f3mem_t *m, uint32_t obj);  /* ROM 0x0CEE84: fn = 0xCEE8C */
int Obj_Dying3_Tick(const f3mem_t *m, uint32_t obj);  /* ROM 0x0CEE8C: Obj_Free */

#endif
