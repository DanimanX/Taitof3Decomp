/* ROM 0x179946: register-level wrapper of Obj_AnimTick idx 6 (src/objects/obj_anim.c) for its capture.
 * This is the 0x179A5C index-6 tick; src/enemy/obj_179632.c carries the same
 * body as its sub_179946 (the 0x179632 init, which bra.w's here).
 *
 * Entry: A0 = obj. Body:
 *   0x179946  bfextu $20(a0){0:8},d0
 *   0x17994C  addq.b #8,d0
 *   0x17994E  lsr.w  #4,d0
 *   0x179950  movea.l $3a(a0),a1
 *   0x179954  mulu.w #$e,d0
 *   0x179958  lea.l  (a1,d0.w),a1
 *   0x17995C  move.l (a1)+,$10(a0)
 *   0x179960  move.l (a1)+,$2c(a0)
 *   0x179964  move.l (a1)+,$30(a0)
 *   0x179968  bfexts (a1){0:8},d0      -- signed byte, no postincrement
 *   0x17996C  bfexts (a1){8:8},d1      -- signed byte at +1
 *   0x179970  add.w  d0,$14(a0)
 *   0x179974  add.w  d1,$18(a0)
 *   0x179978  rts
 * so at the rts D0/D1 are the two record delta bytes sign-extended to 32 bits
 * and A1 is 0x0C past the record start (three longs consumed). A0 and every
 * other register are untouched. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"

int Fn_179946(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t rec = rd32(m, obj + 0x3Au);                      /* 0x179950 */
    uint16_t d0 = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 8u) >> 4); /* 0x179946..0x17994E */
    uint32_t a1 = rec + (uint32_t)(int32_t)(int16_t)(uint16_t)((uint32_t)d0 * 0x0Eu)
                + 0x0Cu;                                      /* 0x179958 + three longs */

    if (Obj_AnimTick(m, obj, 6u) < 0)                         /* ROM 0x179946 */
        return -1;

    r->d[0] = (uint32_t)(int32_t)(int8_t)rd8(m, a1);           /* 0x179968 bfexts (a1){0:8},d0 */
    r->d[1] = (uint32_t)(int32_t)(int8_t)rd8(m, a1 + 1u);      /* 0x17996C bfexts (a1){8:8},d1 */
    r->a[1] = a1;                                             /* 0x179978 rts */
    return 0;
}
