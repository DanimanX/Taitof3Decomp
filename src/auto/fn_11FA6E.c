/* ROM 0x11FA6E: boss/stage phase timer & bitfield trigger tick (A0 = object).
 * Decrements +0x06; if <= 0 transitions to 0x11F7BE. Otherwise if +0x3A == 0,
 * checks frame-rate-compensated modulo timer 0x0CC2BC(0x10); if rem == 0, sets
 * bits 0 and 1 of A5+0x30EE. Then increments +0x3A (setting bit 10 of A5+0x30EE
 * when it reaches 0x10), clears +0x3A if bit 0 of A5+0x30EE is clear, and re-arms
 * 0x11FA6E. */
#include "fn_auto.h"

/* ROM 0x0CC2BC: D0 = (quot << 16) | rem of (A5+0x878) / max(1, ((0x180 - A5+0x87E)*d0w) >> 8). */
static void sub_0CC2BC(const f3mem_t *m, uint16_t d0w, uint32_t *out_d0, uint32_t *out_d1)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu)); /* 0x0CC2BC..0x0CC2C0 */
    uint32_t d1 = ((uint32_t)d0w * (uint32_t)d1w) << 8;    /* 0x0CC2C4..0x0CC2C6 */
    d1 = (d1 >> 16) | (d1 << 16);                           /* 0x0CC2C8 */
    if ((uint16_t)d1 == 0u)                                 /* 0x0CC2CA..0x0CC2CC */
        d1 = 1u;                                            /* 0x0CC2D0 */
    uint32_t dividend = rd16(m, F3_A5 + 0x0878u);           /* 0x0CC2D2..0x0CC2D4 */
    uint16_t divisor = (uint16_t)d1;
    uint16_t quot = (uint16_t)(dividend / divisor);         /* 0x0CC2D8 */
    uint16_t rem = (uint16_t)(dividend % divisor);
    *out_d0 = ((uint32_t)quot << 16) | rem;                 /* 0x0CC2DA swap D0 */
    *out_d1 = d1;
}

int Fn_11FA6E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t timer = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x11FA6E */
    wr16(m, obj + 0x06u, timer);
    if ((int16_t)timer <= 0) {                              /* 0x11FA72 -> 0x11FABA -> 0x11F7B6 */
        wr32(m, obj, 0x0011F7BEu);
        return 0;
    }

    if (rd8(m, obj + 0x3Au) == 0u) {                        /* 0x11FA76..0x11FA7A */
        uint32_t d0, d1;
        sub_0CC2BC(m, 0x0010u, &d0, &d1);                   /* 0x11FA7E..0x11FA82 */
        r->d[0] = d0;
        r->d[1] = d1;
        if ((uint16_t)d0 != 0u) {                           /* 0x11FA88..0x11FA8A */
            wr32(m, obj, 0x0011FA6Eu);                      /* 0x11FA66 */
            return 0;
        }
        /* 0x11FA8C..0x11FA92: bfset (0x30ee,A5){0:1} and {1:1} */
        wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0xC0u));
    }

    uint8_t c = (uint8_t)(rd8(m, obj + 0x3Au) + 1u);        /* 0x11FA98 */
    wr8(m, obj + 0x3Au, c);
    if (c == 0x10u)                                         /* 0x11FA9C..0x11FAA2 */
        wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x20u)); /* 0x11FAA6 {0xa:1} */

    if ((rd8(m, F3_A5 + 0x30EEu) & 0x80u) == 0u)            /* 0x11FAAC..0x11FAB2 {0x0:1} */
        wr8(m, obj + 0x3Au, 0u);                            /* 0x11FAB4 */

    wr32(m, obj, 0x0011FA6Eu);                              /* 0x11FAB8 -> 0x11FA66 */
    return 0;
}
