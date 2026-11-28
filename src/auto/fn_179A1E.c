/* ROM 0x179A1E: register-level wrapper of Obj_AnimTick idx 16 (src/objects/obj_anim.c) for its capture.
 *
 * Entry: A0 = obj. Body:
 *   0x179A1E  bfextu $20(a0){0:8},d0
 *   0x179A24  subi.b #$20,d0           -- sector byte [0x20..0x60]
 *   0x179A28  cmpi.b #$0,d0 ; 0x179A2C bge.w 0x179A34
 *   0x179A30  move.b #$0,d0
 *   0x179A34  cmpi.b #$40,d0 ; 0x179A38 ble.w 0x179A40
 *   0x179A3C  move.b #$40,d0
 *   0x179A40  addq.b #2,d0
 *   0x179A42  lsr.w  #2,d0             -- 16 clamped steps
 *   0x179A44  movea.l $3a(a0),a1
 *   0x179A48  mulu.w #$6,d0
 *   0x179A4C  lea.l  (a1,d0.w),a1
 *   0x179A50  move.l (a1)+,$10(a0)
 *   0x179A54  move.w (a1)+,d0          -- overwrites the mulu result
 *   0x179A56  move.b d0,$1c(a0)
 *   0x179A5A  rts
 * so at the rts D0 is the record's +4 palette word (move.w clears the upper
 * half) and A1 is 6 past the record start (4+2). A0 and every other register
 * are untouched. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"

int Fn_179A1E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t rec = rd32(m, obj + 0x3Au);                      /* 0x179A44 */
    int8_t b = (int8_t)(uint8_t)(rd8(m, obj + 0x20u) - 0x20u); /* 0x179A1E..0x179A24 */

    if (b < 0)                                                 /* 0x179A28 bge */
        b = 0;                                                 /* 0x179A30 */
    if (b > 0x40)                                              /* 0x179A34 ble */
        b = 0x40;                                              /* 0x179A3C */

    uint16_t d0 = (uint16_t)((uint8_t)(b + 2) >> 2);           /* 0x179A40..0x179A42 */
    uint32_t base = rec + (uint32_t)(int32_t)(int16_t)(uint16_t)((uint32_t)d0 * 6u);

    if (Obj_AnimTick(m, obj, 16u) < 0)                         /* ROM 0x179A1E */
        return -1;

    r->d[0] = rd16(m, base + 4u);                              /* 0x179A54 move.w (a1)+,d0 */
    r->a[1] = base + 6u;                                       /* 0x179A5A rts */
    return 0;
}
