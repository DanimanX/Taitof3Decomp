#include "obj_1316D2.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"

/* ROM 0x0CC2BC: frame-rate scaled probability gate. Threshold D0w becomes
 * ((0x180 - A5+0x87E) * D0w) << 8, swapped; divu of A5+0x878 by it leaves the
 * remainder in the high word; `swap D0` leaves the remainder in D0w, which
 * the callers test. */
static uint16_t sub_0CC2BC(const f3mem_t *m, uint16_t d0w)
{
    uint32_t d1 = (uint32_t)(uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    d1 = d1 * (uint32_t)d0w;                           /* mulu.w: 32-bit product */
    d1 <<= 8;                                          /* lsl.l #8 */
    d1 = (d1 << 16) | (d1 >> 16);                      /* swap */
    uint16_t thr = (uint16_t)d1;
    if (thr == 0u)
        thr = 1u;                                      /* 0x0CC2D0 */
    return (uint16_t)(rd16(m, F3_A5 + 0x0878u) % thr); /* divu + swap */
}

/* ROM 0x0CC33A: aim from this object toward the object at A5+0x1D28, offset by
 * (-D0w,-D1w). With no target it is the fixed 0x80. */
static uint16_t sub_0CC33A(const f3mem_t *m, uint32_t obj, uint16_t d0w,
                           uint16_t d1w)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x1D28u);
    if (a1 == 0u)
        return 0x0080u;                                /* 0x0CC342 */

    uint16_t t = (uint16_t)(rd16(m, a1 + 0x2Cu) + rd16(m, a1 + 0x2Eu));
    t = (uint16_t)((int16_t)t >> 1);                   /* asr.w #1 */
    d0w = (uint16_t)(0u - d0w);                        /* neg.w D0 */
    d0w = (uint16_t)(d0w + t);
    d0w = (uint16_t)(d0w + rd16(m, a1 + 0x14u));
    d0w = (uint16_t)(d0w - rd16(m, obj + 0x14u));

    t = (uint16_t)(rd16(m, a1 + 0x30u) + rd16(m, a1 + 0x32u));
    t = (uint16_t)((int16_t)t >> 1);                   /* asr.w #1 */
    d1w = (uint16_t)(0u - d1w);                        /* neg.w D1 */
    d1w = (uint16_t)(d1w + t);
    d1w = (uint16_t)(d1w + rd16(m, a1 + 0x18u));
    d1w = (uint16_t)(d1w - rd16(m, obj + 0x18u));

    return Obj_Atan2(m, d0w, d1w);                     /* 0x0CC37C -> 0x006422 */
}

/* ROM 0x1316D2: on a 9/0x180-scaled roll, spawn one 0x0CCD04 (or 0x0CCDB6 once
 * the scroll counter A5+0x87E reaches 0x18) in group 9, two groups wide, with
 * arg = 0x08080000 | ((angle + (rng&0x1F) - 0x10)&0xFF)<<8, then clear
 * its own anim fields +0x3A/+0x3E. Either way +0x22 counts up and +0x06 steps
 * once it reaches 0xB4. */
int Obj_1316D2_Callback(const f3mem_t *m, uint32_t obj)
{
    /* 0x1316D2: move.w #9,D0 ; jsr 0x0CC2BC ; tst.w D0 ; bne 0x131744 */
    if (sub_0CC2BC(m, 0x0009u) != 0u)
        goto tick_131744;

    /* 0x1316E2: D0w = D1w = 8 -> 0x0CC33A ; move.b D0w,D1w */
    {
        uint16_t d1 = sub_0CC33A(m, obj, 0x0008u, 0x0008u) & 0xFFu;

        /* 0x1316F4: jsr 0x001D8A ; andi.b #0x1F,D0 ; subi.b #0x10,D0 */
        uint8_t r = (uint8_t)(((uint8_t)Rng_NextMem(m) & 0x1Fu) - 0x10u);
        d1 = (uint8_t)(d1 + r);                        /* add.b D0,D1 */
        d1 = (uint16_t)(d1 << 8);                      /* lsl.w #8 */
        /* ori.l #0x08080000,D1 only sets the high word (0x0808) */

        /* 0x13170C: cmpi.w #0x18,A5+0x87E ; bge 0x131720 */
        uint32_t fn = 0x000CCD04u;
        if ((int16_t)rd16(m, F3_A5 + 0x087Eu) >= 0x18)
            fn = 0x000CCDB6u;                          /* 0x131720 */

        /* 0x131726: push arg, 2, 9, fn */
        (void)Obj_Spawn(m, obj, fn, 9u, 2u, 0x08080000u | (uint32_t)d1);

        /* 0x13173C: (0x3A,A0) / (0x3E,A0) after 0x01057C restored A0 from its
         * saved D4, so these clear the CALLER's fields, not the child's. */
        wr16(m, obj + 0x3Au, 0u);
        wr16(m, obj + 0x3Eu, 0u);
    }

tick_131744:
    /* 0x131744: addq.w #1,(0x22,A0) ; cmpi.w #0xB4 ; blt 0x131756 */
    {
        uint16_t t = (uint16_t)(rd16(m, obj + 0x22u) + 1u);
        wr16(m, obj + 0x22u, t);
        if ((int16_t)t >= 0x00B4)
            wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) + 1u));
    }
    return 0;
}
