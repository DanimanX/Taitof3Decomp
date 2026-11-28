/* ROM 0x1799B2: register-level wrapper of Obj_AnimTick idx 7 (src/objects/obj_anim.c) for its capture.
 * This is the 0x179A5C index-7 tick; src/enemy/obj_179646.c carries the same
 * body as its sub_1799B2 (the 0x179646 init, which bra.w's here).
 *
 * Entry: A0 = obj. Body:
 *   0x1799B2  bfextu $20(a0){0:8},d0
 *   0x1799B8  addq.b #4,d0             -- finer angle: >>3
 *   0x1799BA  lsr.w  #3,d0
 *   0x1799BC  movea.l $3a(a0),a1
 *   0x1799C0  mulu.w #$e,d0
 *   0x1799C4  lea.l  (a1,d0.w),a1
 *   0x1799C8  move.l (a1)+,$10(a0)
 *   0x1799CC  move.l (a1)+,$2c(a0)
 *   0x1799D0  move.l (a1)+,$30(a0)
 *   0x1799D4  bfexts (a1){0:8},d0      -- signed byte, no postincrement
 *   0x1799D8  bfexts (a1){8:8},d1      -- signed byte at +1
 *   0x1799DC  add.w  d0,$14(a0)
 *   0x1799E0  add.w  d1,$18(a0)
 *   0x1799E4  rts
 * so at the rts D0/D1 are the two record delta bytes sign-extended to 32 bits
 * and A1 is 0x0C past the record start (three longs consumed). A0 and every
 * other register are untouched. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"

int Fn_1799B2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t rec = rd32(m, obj + 0x3Au);                      /* 0x1799BC */
    uint16_t d0 = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 4u) >> 3); /* 0x1799B2..0x1799BA */
    uint32_t a1 = rec + (uint32_t)(int32_t)(int16_t)(uint16_t)((uint32_t)d0 * 0x0Eu)
                + 0x0Cu;                                      /* 0x1799C4 + three longs */

    if (Obj_AnimTick(m, obj, 7u) < 0)                         /* ROM 0x1799B2 */
        return -1;

    r->d[0] = (uint32_t)(int32_t)(int8_t)rd8(m, a1);           /* 0x1799D4 bfexts (a1){0:8},d0 */
    r->d[1] = (uint32_t)(int32_t)(int8_t)rd8(m, a1 + 1u);      /* 0x1799D8 bfexts (a1){8:8},d1 */
    r->a[1] = a1;                                             /* 0x1799E4 rts */
    return 0;
}
