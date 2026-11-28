/* ROM 0x005794: UNVERIFIED (weak capture): write the decimal digits of a
 * 16-bit value into a buffer backwards, then pad with a fill byte.
 *   arg1.w (SP+4)  value to print (zero-extended into D7)
 *   arg2.l (SP+6)  buffer pointer
 *   arg4.w (SP+0xA) padding byte (D1.b)
 *   arg3.w (SP+0xC) end index: buffer[index] = 0, digits are written at
 *                   index-1, index-2, ... until the quotient is 0 or the
 *                   index-1 counter runs out, then padding fills the rest.
 * The routine saves D1/D2/D7/A0, so it returns every register unchanged; the
 * capture only ever shows the digits-skipped / padding-skipped path (arg3.w =
 * 0, arg4.w odd) and its buffer pointer is outside the captured RAM. */
#include "fn_auto.h"

int Fn_005794(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    /* movem.l {A0 D7 D2 D1},-(SP) at 0x005798 saves all four again, so the
     * only lasting effects are the buffer bytes. */
    uint32_t d1_saved = r->d[1], d2_saved = r->d[2];
    uint32_t d7_saved = r->d[7], a0_saved = r->a[0];

    uint32_t d7 = (uint32_t)rd16(m, sp + 4u);                 /* 0x00579C..0x00579E */
    uint32_t a0 = rd32(m, sp + 6u)
                + (uint32_t)(int32_t)(int16_t)rd16(m, sp + 0x0Au); /* 0x0057A2..0x0057AA */
    uint16_t d2 = rd16(m, sp + 0x0Cu);                        /* 0x0057A6 (0x10,A6) */
    uint32_t d1 = 10;                                         /* 0x0057AE moveq #0xa */

    wr8(m, a0, 0u);                                           /* 0x0057AC clr.b (A0) */
    d2 = (uint16_t)(d2 - 1u);                                 /* 0x0057B0 */
    if ((int16_t)d2 > 0) {                                    /* 0x0057B2 ble 0x57C6 */
        for (;;) {
            /* 0x0057B4 divu.w D1w,D7: quotient low word, remainder high */
            uint32_t dv = d7;
            uint32_t quo = dv / (d1 & 0xFFFFu);
            uint32_t rem = dv % (d1 & 0xFFFFu);
            if (quo <= 0xFFFFu)                               /* D7 unchanged on overflow */
                d7 = (rem << 16) | quo;
            d7 = (d7 << 16) | (d7 >> 16);                     /* 0x0057B6 swap D7 */
            uint8_t digit = (uint8_t)((d7 & 0xFFu) + 0x30u);  /* 0x0057B8 addi.b #0x30 */
            d7 = (d7 & 0xFFFFFF00u) | digit;
            a0 -= 1u;                                         /* 0x0057BC move.b D7b,-(A0) */
            wr8(m, a0, digit);
            d7 &= 0xFFFF0000u;                                /* 0x0057BE clr.w D7w */
            d7 = (d7 << 16) | (d7 >> 16);                     /* 0x0057C0 swap D7 */
            if (d7 == 0u)                                     /* 0x0057C2 dbeq: Z set */
                break;
            d2 = (uint16_t)(d2 - 1u);
            if ((int16_t)d2 == -1)
                break;
        }
    }

    d1 = rd16(m, sp + 0x0Au);                                 /* 0x0057C6 move.w (0xe,A6),D1w */
    for (;;) {
        d2 = (uint16_t)(d2 - 1u);                             /* 0x0057CA subq.w #1,D2w */
        if ((int16_t)d2 < 0)                                  /* 0x0057CC blt 0x57D2 */
            break;
        a0 -= 1u;                                             /* 0x0057CE move.b D1b,-(A0) */
        wr8(m, a0, (uint8_t)d1);
    }

    r->d[1] = d1_saved;                                       /* 0x0057D2 movem.l (SP)+ */
    r->d[2] = d2_saved;
    r->d[7] = d7_saved;
    r->a[0] = a0_saved;
    r->a[7] = sp + 4u;                                        /* unlk A6 ; rts */
    return 0;
}
