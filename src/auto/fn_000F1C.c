/* ROM 0x000F1C: five-frame service-output blink for one coin channel.
 * A0 = channel record, A4 = output byte, D1 = output bit number:
 *
 *   000F1C  addq.b #1,(A0+2) ; D0 = that byte ; D0 -= 5
 *   000F26  if D0 < 0: bset.b D1,(A4)                          ; frames 1..5
 *   000F2C  D0 -= 5
 *   000F2E  if D0 < 0: bclr.b D1,(A4)                          ; frames 6..10
 *   000F34  (byte -= 0x10: positive) clr.b (A0+2) ; subq.b #1,(A0+1)
 *   000F3C  rts
 *
 * Only one 1-pair capture (the addq path, counter 0x18 -> 0x19, two decrements
 * to 0x0F, then the 0x0F34 reset); the bset/bclr branches were re-read from
 * the bytes. */
#include "fn_auto.h"

int Fn_000F1C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0], a4 = r->a[4];
    uint8_t counter = (uint8_t)(rd8(m, a0 + 2u) + 1u);   /* 0x000F1C addq.b #1 */
    uint8_t result;
    uint8_t bit;

    wr8(m, a0 + 2u, counter);
    result = (uint8_t)(rd8(m, a0 + 2u) - 5u);            /* 0x000F20/0x000F24 */
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | result;
    if ((int8_t)result < 0) {                            /* 0x000F26 bge 0x0F2C */
        bit = (uint8_t)(1u << (r->d[1] & 7u));           /* 0x000F28 bset.b D1,(A4) */
        wr8(m, a4, (uint8_t)(rd8(m, a4) | bit));
        return 0;                                        /* 0x000F2A bra 0x0F3C */
    }

    result = (uint8_t)(result - 5u);                     /* 0x000F2C */
    r->d[0] = (r->d[0] & 0xFFFFFF00u) | result;
    if ((int8_t)result < 0) {                            /* 0x000F2E bge 0x0F34 */
        bit = (uint8_t)(1u << (r->d[1] & 7u));           /* 0x000F30 bclr.b D1,(A4) */
        wr8(m, a4, (uint8_t)(rd8(m, a4) & (uint8_t)~bit));
        return 0;                                        /* 0x000F32 bra 0x0F3C */
    }

    wr8(m, a0 + 2u, 0u);                                 /* 0x000F34 clr.b (A0+2) */
    wr8(m, a0 + 1u, (uint8_t)(rd8(m, a0 + 1u) - 1u));    /* 0x000F38 subq.b #1,(A0+1) */
    return 0;
}
