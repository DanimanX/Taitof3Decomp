/* ROM 0x1798F8: register-level wrapper of Obj_AnimTick idx 4 (src/objects/obj_anim.c) for its capture.
 * This is the 0x179A5C index-4 tick; src/enemy/obj_17961E.c carries the same
 * body as its scr_dir16_box (the 0x17961E init, which bra.w's here).
 *
 * Entry: A0 = obj. Body:
 *   0x1798F8  bfextu $20(a0){0:8},d0
 *   0x1798FE  addq.b #8,d0
 *   0x179900  lsr.w  #4,d0
 *   0x179902  movea.l $3a(a0),a1
 *   0x179906  mulu.w #$c,d0            -- full 32-bit product into D0
 *   0x17990A  lea.l  (a1,d0.w),a1
 *   0x17990E  move.l (a1)+,$10(a0)
 *   0x179912  move.l (a1)+,$2c(a0)
 *   0x179916  move.l (a1)+,$30(a0)
 *   0x17991A  rts
 * so at the rts D0 is the frame index times 12 (mulu.w leaves the whole 32-bit
 * product, which stays below 0x10000 here) and A1 is that many bytes past the
 * record start, plus the three longs the body consumed (0x0C). A0 and every
 * other register are untouched. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"

int Fn_1798F8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t rec = rd32(m, obj + 0x3Au);                      /* 0x179902 */
    uint16_t d0 = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 8u) >> 4); /* 0x1798F8..0x179900 */
    uint32_t off = (uint32_t)d0 * 0x0Cu;                      /* 0x179906 mulu.w #$c,d0 */

    if (Obj_AnimTick(m, obj, 4u) < 0)                         /* ROM 0x1798F8 */
        return -1;

    r->d[0] = off;                                            /* 0x179906 */
    r->a[1] = rec + (uint32_t)(int32_t)(int16_t)(uint16_t)off + 0x0Cu; /* 0x17990A + 0x0E..0x916 */
    return 0;
}
