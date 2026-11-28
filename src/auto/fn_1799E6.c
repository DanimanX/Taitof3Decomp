/* ROM 0x1799E6: register-level wrapper of Obj_AnimTick idx 15 (src/objects/obj_anim.c) for its capture.
 *
 * Entry: A0 = obj. Body:
 *   0x1799E6  bfextu $20(a0){0:8},d0
 *   0x1799EC  addq.b #4,d0
 *   0x1799EE  lsr.w  #3,d0
 *   0x1799F0  movea.l $3a(a0),a1
 *   0x1799F4  lsl.w  #4,d0             -- 16-byte records (index << 4)
 *   0x1799F6  lea.l  (a1,d0.w),a1
 *   0x1799FA  move.l (a1)+,$10(a0)
 *   0x1799FE  move.w (a1)+,d0
 *   0x179A00  move.b d0,$1c(a0)
 *   0x179A04  move.l (a1)+,$2c(a0)
 *   0x179A08  move.l (a1)+,$30(a0)
 *   0x179A0C  bfexts (a1){0:8},d0      -- signed byte, no postincrement
 *   0x179A10  bfexts (a1){8:8},d1      -- signed byte at +1
 *   0x179A14  add.w  d0,$14(a0)
 *   0x179A18  add.w  d1,$18(a0)
 *   0x179A1C  rts
 * so at the rts D0/D1 are the two record delta bytes sign-extended to 32 bits
 * (D0's palette word is overwritten by the bfexts) and A1 is 0x0E past the
 * record start (4+2+4+4). A0 and every other register are untouched. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"

int Fn_1799E6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t rec = rd32(m, obj + 0x3Au);                      /* 0x1799F0 */
    uint16_t d0 = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 4u) >> 3); /* 0x1799E6..0x1799EE */
    uint16_t sel = (uint16_t)(d0 << 4);                       /* 0x1799F4 lsl.w #$4,d0 */
    uint32_t base = rec + (uint32_t)(int32_t)(int16_t)sel;    /* 0x1799F6 lea.l (a1,d0.w),a1 */

    if (Obj_AnimTick(m, obj, 15u) < 0)                        /* ROM 0x1799E6 */
        return -1;

    r->d[0] = (uint32_t)(int32_t)(int8_t)rd8(m, base + 0x0Eu); /* 0x179A0C bfexts (a1){0:8},d0 */
    r->d[1] = (uint32_t)(int32_t)(int8_t)rd8(m, base + 0x0Fu); /* 0x179A10 bfexts (a1){8:8},d1 */
    r->a[1] = base + 0x0Eu;                                   /* 0x179A1C rts */
    return 0;
}
