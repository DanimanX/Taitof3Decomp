/* ROM 0x17991C: register-level wrapper of Obj_AnimTick idx 12 (src/objects/obj_anim.c) for its capture.
 *
 * Entry: A0 = obj. Body:
 *   0x17991C  bfextu $20(a0){0:8},d0
 *   0x179922  addq.b #8,d0
 *   0x179924  lsr.w  #4,d0
 *   0x179926  movea.l $3a(a0),a1
 *   0x17992A  mulu.w #$e,d0
 *   0x17992E  lea.l  (a1,d0.w),a1
 *   0x179932  move.l (a1)+,$10(a0)
 *   0x179936  move.w (a1)+,d0          -- overwrites the mulu result
 *   0x179938  move.b d0,$1c(a0)
 *   0x17993C  move.l (a1)+,$2c(a0)
 *   0x179940  move.l (a1)+,$30(a0)
 *   0x179944  rts
 * so at the rts D0 is the record's +4 palette word (move.w clears the upper
 * half) and A1 is 0x0E past the record start (4+2+4+4). A0 and every other
 * register are untouched. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"

int Fn_17991C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t rec = rd32(m, obj + 0x3Au);                      /* 0x179926 */
    uint16_t d0 = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 8u) >> 4); /* 0x17991C..0x179924 */
    uint32_t base = rec + (uint32_t)(int32_t)(int16_t)(uint16_t)((uint32_t)d0 * 0x0Eu);

    if (Obj_AnimTick(m, obj, 12u) < 0)                        /* ROM 0x17991C */
        return -1;

    r->d[0] = rd16(m, base + 4u);                             /* 0x179936 move.w (a1)+,d0 */
    r->a[1] = base + 0x0Eu;                                   /* 0x179944 rts */
    return 0;
}
