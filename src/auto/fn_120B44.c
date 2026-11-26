/* ROM 0x120B44: delayed falling hazard spawn/init tick (A0 = object).
 * Decrements +0x06 and re-arms 0x120B44 while > 0. On <= 0, picks random
 * X (+0x14.w) in [0..255] via 0x001D8A, scales speed 0x12 by difficulty
 * via 0x0CBEB0 into +0x42, sets +0x20 = 0x40 (down), and transitions to
 * 0x120B74. */
#include "fn_auto.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CBEB0: difficulty-scaled speed word (A5+0x87E). */
static uint32_t scale_speed_0CBEB0(const f3mem_t *m, uint32_t d0, uint32_t *d1_io)
{
    uint16_t d1w = rd16(m, F3_A5 + 0x087Eu);                 /* 0x0CBEB0 */
    d1w = (uint16_t)(d1w << 3);                              /* 0x0CBEB4 */
    d1w = (uint16_t)(d1w + 0x0C00u);                         /* 0x0CBEB6 */
    *d1_io = setw(*d1_io, d1w);
    int32_t prod = (int32_t)(int16_t)(uint16_t)d0 * (int32_t)(int16_t)d1w; /* 0x0CBEBA */
    uint32_t shifted = (uint32_t)prod << 4;                  /* 0x0CBEBC */
    return (shifted << 16) | (shifted >> 16);                /* 0x0CBEBE swap */
}

int Fn_120B44(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);    /* 0x120B44 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                  /* 0x120B48 */
        wr32(m, obj, 0x00120B44u);                           /* 0x120B3C */
        return 0;
    }

    uint32_t d0 = Rng_NextMem(m);                            /* 0x120B4A -> 0x001D8A */
    d0 = setw(d0, (uint16_t)(d0 & 0x00FFu));                 /* 0x120B50 */
    wr16(m, obj + 0x14u, (uint16_t)d0);                      /* 0x120B54 */

    d0 = setw(d0, 0x0012u);                                  /* 0x120B58 */
    d0 = scale_speed_0CBEB0(m, d0, &r->d[1]);                /* 0x120B5C -> 0x0CBEB0 */
    wr16(m, obj + 0x42u, (uint16_t)d0);                      /* 0x120B62 */
    wr16(m, obj + 0x20u, 0x0040u);                           /* 0x120B66 */
    wr32(m, obj, 0x00120B74u);                               /* 0x120B6C */
    r->d[0] = d0;
    return 0;
}
