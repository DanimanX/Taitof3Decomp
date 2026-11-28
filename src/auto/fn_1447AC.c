/* ROM 0x1447AC: UNVERIFIED (no usable capture - the only pair is
 * VBlank-interrupted and the test harness rejects it): insertion sort of the 8-byte records at 0x40C2B4, keyed on
 * their first word (ascending, signed), ending at the first record whose word
 * at +0x06 is negative. Walks A0 by 8, and for a record smaller than its
 * predecessor saves the 8 bytes on the stack, shifts the greater predecessors
 * right by 8 (0x1447F0) and drops the saved record into the hole. The walk
 * stops at the negative +0x06 marker. */
#include "fn_auto.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_1447AC(const f3mem_t *m, f3regs_t *r)
{
    const uint32_t base = 0x0040C2B4u;                   /* 0x1447AC lea */
    uint32_t a0 = base;
    uint32_t a1 = r->a[1];
    uint32_t a2 = r->a[2];
    uint32_t d0 = r->d[0];

    for (;;) {
        if ((int16_t)rd16(m, a0 + 6u) < 0)               /* 0x1447B2 tst.w + bmi */
            break;                                       /* 0x144828 rts */

        a0 += 8u;                                        /* 0x1447BA */
        d0 = setw(d0, rd16(m, a0));                      /* 0x1447BE */
        if ((int16_t)rd16(m, a0) >= (int16_t)rd16(m, a0 - 8u))
            continue;                                    /* 0x1447C0 bge 0x1447B2 */

        a1 = a0;                                         /* 0x1447C6 */
        for (;;) {
            a1 -= 8u;                                    /* 0x1447C8 */
            if (a1 <= base)                              /* 0x1447CC cmpa.l #0x40C2B4 */
                break;                                   /* 0x1447D2 ble 0x1447DC */
            if ((int16_t)d0 >= (int16_t)rd16(m, a1 - 8u))
                break;                                   /* 0x1447DA blt 0x1447C8 not taken */
        }                                                /* 0x1447DC */

        a2 = a0;                                         /* 0x1447DC */
        uint16_t w0 = rd16(m, a2);                       /* 0x1447DE..0x1447EC save 8 bytes */
        uint16_t w1 = rd16(m, a2 + 2u);
        uint16_t w2 = rd16(m, a2 + 4u);
        uint8_t b0 = rd8(m, a2 + 6u);
        uint8_t b1 = rd8(m, a2 + 7u);

        do {                                             /* 0x1447F0 */
            wr16(m, a2, rd16(m, a2 - 8u));
            wr16(m, a2 + 2u, rd16(m, a2 - 6u));
            wr16(m, a2 + 4u, rd16(m, a2 - 4u));
            wr8(m, a2 + 6u, rd8(m, a2 - 2u));
            wr8(m, a2 + 7u, rd8(m, a2 - 1u));
            a2 -= 8u;                                    /* 0x14480C */
        } while (a2 > a1);                               /* 0x144810..0x144812 bgt */

        wr8(m, a2 + 7u, b1);                             /* 0x144814..0x144824 */
        wr8(m, a2 + 6u, b0);
        wr16(m, a2 + 4u, w2);
        wr16(m, a2 + 2u, w1);
        wr16(m, a2, w0);                                 /* 0x144826 bra 0x1447B2 */
    }

    r->d[0] = d0;
    r->a[0] = a0;
    r->a[1] = a1;
    r->a[2] = a2;
    return 0;
}
