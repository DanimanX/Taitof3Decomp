/* ROM 0x11CEEE: stage-event countdown stage 1 (A0 = object). Decrements +0x06
 * and re-arms while it stays positive; otherwise sets A5+0x30EE bit 6 (0x40),
 * reloads +0x06 with 0x64 - 0x0CBEB0(0x46) and transitions to 0x11CF18, or -
 * when that reload lands on zero - sets A5+0x30EE bit 3 (0x10), reloads +0x06
 * with 0x8C and transitions straight to 0x11CF34. */
#include "fn_auto.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CBEB0: D0 = swap((D0.w * ((A5+0x87E << 3) + 0xC00)) << 4); D1.w gets
 * the scaled factor. The caller uses only the low word, but the swapped high
 * word survives in D0 as well. */
static void sub_0CBEB0(const f3mem_t *m, f3regs_t *r)
{
    uint16_t d1w = (uint16_t)((rd16(m, F3_A5 + 0x087Eu) << 3) + 0x0C00u); /* 0x0CBEB0..0x0CBEB6 */
    r->d[1] = setw(r->d[1], d1w);
    uint32_t prod = (uint32_t)((int32_t)(int16_t)(uint16_t)r->d[0] * (int32_t)(int16_t)d1w);
    prod <<= 4;                                                             /* 0x0CBEBC asl.l */
    r->d[0] = (prod << 16) | (prod >> 16);                                  /* 0x0CBEBE swap */
}

int Fn_11CEEE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);                    /* 0x11CEEE */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                                 /* 0x11CEF2 */
        wr32(m, obj, 0x0011CEEEu);                                          /* 0x11CEE6 */
        return 0;
    }

    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x40u));     /* 0x11CEF4 {1:1} */
    r->d[0] = setw(r->d[0], 0x0046u);                                       /* 0x11CEFA */
    sub_0CBEB0(m, r);                                                       /* 0x11CEFE */
    uint16_t w = (uint16_t)((uint16_t)(0u - (uint16_t)r->d[0]) + 0x0064u);  /* 0x11CF04..0x11CF06 */
    r->d[0] = setw(r->d[0], w);
    wr16(m, obj + 0x06u, w);                                                /* 0x11CF0A */
    if (w == 0u) {                                                          /* 0x11CF0E */
        wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x10u)); /* 0x11CF1E {3:1} */
        wr16(m, obj + 0x06u, 0x008Cu);                                      /* 0x11CF24 (0x11CF2A beq folded) */
        wr32(m, obj, 0x0011CF34u);                                          /* 0x11CF2C */
        return 0;
    }
    wr32(m, obj, 0x0011CF18u);                                              /* 0x11CF10 */
    return 0;
}
