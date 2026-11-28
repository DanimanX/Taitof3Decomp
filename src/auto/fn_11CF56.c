/* ROM 0x11CF56: stage-event countdown stage 3 (A0 = object). Decrements +0x06
 * and re-arms while it stays positive; otherwise clears A5+0x30EE bit 3 (0x10),
 * reloads +0x06 with 0xA0 - 0x0CBEB0(0x64) and transitions to 0x11CF80, or -
 * when that reload lands on zero - clears A5+0x30EE bit 1 (0x40), sets
 * A5+0x30F2 bit 1 (0x02) and A5+0x30EE bit 6 (0x02), clears bit 5, loads
 * +0x06 = 0x12C, randomises +0x3A (0xDC..0xFB) and +0x3C (0x40..0xBF) from
 * Rng_Next (0x001D8A) and transitions to 0x11CE6E. */
#include "fn_auto.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CBEB0: D0 = swap((D0.w * ((A5+0x87E << 3) + 0xC00)) << 4); D1.w gets
 * the scaled factor. */
static void sub_0CBEB0(const f3mem_t *m, f3regs_t *r)
{
    uint16_t d1w = (uint16_t)((rd16(m, F3_A5 + 0x087Eu) << 3) + 0x0C00u); /* 0x0CBEB0..0x0CBEB6 */
    r->d[1] = setw(r->d[1], d1w);
    uint32_t prod = (uint32_t)((int32_t)(int16_t)(uint16_t)r->d[0] * (int32_t)(int16_t)d1w);
    prod <<= 4;                                                             /* 0x0CBEBC asl.l */
    r->d[0] = (prod << 16) | (prod >> 16);                                  /* 0x0CBEBE swap */
}

int Fn_11CF56(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);                    /* 0x11CF56 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                                 /* 0x11CF5A */
        wr32(m, obj, 0x0011CF56u);                                          /* 0x11CF4E */
        return 0;
    }

    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & ~0x10u));   /* 0x11CF5C {3:1} */
    r->d[0] = setw(r->d[0], 0x0064u);                                       /* 0x11CF62 */
    sub_0CBEB0(m, r);                                                       /* 0x11CF66 */
    uint16_t w = (uint16_t)((uint16_t)(0u - (uint16_t)r->d[0]) + 0x00A0u);  /* 0x11CF6C..0x11CF6E */
    r->d[0] = setw(r->d[0], w);
    wr16(m, obj + 0x06u, w);                                                /* 0x11CF72 */
    if (w != 0u) {                                                          /* 0x11CF76 */
        wr32(m, obj, 0x0011CF80u);                                          /* 0x11CF78 */
        return 0;
    }

    /* 0x11CF86..0x11CE6C: end of the event. */
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & ~0x40u));   /* 0x11CF86 {1:1} */
    wr8(m, F3_A5 + 0x30F2u, (uint8_t)(rd8(m, F3_A5 + 0x30F2u) | 0x02u));    /* 0x11CE2A bset #1 */
    wr8(m, F3_A5 + 0x30EEu,
        (uint8_t)((rd8(m, F3_A5 + 0x30EEu) | 0x02u) & ~0x04u));             /* 0x11CE30 {6:1}, 0x11CE36 {5:1} */
    wr16(m, obj + 0x06u, 0x012Cu);                                          /* 0x11CE3C */

    r->d[0] = Rng_NextMem(m);                                               /* 0x11CE42 */
    uint16_t a = (uint16_t)(((uint16_t)r->d[0] & 0x1Fu) + 0x00DCu);         /* 0x11CE48..0x11CE4C */
    r->d[0] = setw(r->d[0], a);
    wr16(m, obj + 0x3Au, a);                                                /* 0x11CE50 */

    r->d[0] = Rng_NextMem(m);                                               /* 0x11CE54 */
    uint16_t b = (uint16_t)(((uint16_t)r->d[0] & 0x7Fu) + 0x0040u);         /* 0x11CE5A..0x11CE5E */
    r->d[0] = setw(r->d[0], b);
    wr16(m, obj + 0x3Cu, b);                                                /* 0x11CE62 */

    wr32(m, obj, 0x0011CE6Eu);                                              /* 0x11CE66 */
    return 0;
}
