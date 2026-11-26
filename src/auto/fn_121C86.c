/* ROM 0x121C86: boss/object phase tick (A0 = object).
 * If (int8_t)A5+0x326E >= 6, plays Sound_PlayHooked(0x13), clears A5+0x30EE.l,
 * computes +0x06 = -(0xCBEB0(0x3C) - 0x78), rolls random +0x3A in [0xCD..0xEC]
 * and +0x3C in [0x40..0xBF], and transitions to 0x121DAC.
 * Otherwise integrates A5+0x30D2/30D6 along +0x20 via 0x11B262, advances +0x20
 * by 0x80, decrements +0x06 and re-arms 0x121C86 while > 0; on expiry clears
 * A5+0x30EE bit 4, sets bit 1, sets A5+0x30F2 = 0x22, +0x06 = 0xB4, and
 * transitions to 0x121CC2. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CBEB0: scale signed word D0 by difficulty (A5+0x87E). */
static void sub_0CBEB0(const f3mem_t *m, f3regs_t *r)
{
    uint16_t d1w = (uint16_t)((rd16(m, F3_A5 + 0x087Eu) << 3) + 0x0C00u);
    int32_t prod = (int32_t)(int16_t)(uint16_t)r->d[0] * (int32_t)(int16_t)d1w;
    uint32_t shifted = (uint32_t)prod << 4;
    r->d[0] = (shifted >> 16) | (shifted << 16);
    r->d[1] = setw(r->d[1], d1w);
}

/* ROM 0x11B262: integrate A5+0x30D2/30D6 along +0x20.b at speed +0x42 via 0x0CBE12. */
static void sub_11B262(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x30D2u));
    wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x30D6u));
    uint16_t ang = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    r->d[0] = setw(r->d[0], ang);

    int16_t speed = (int16_t)rd16(m, obj + 0x42u);
    int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + 2u * ang);
    int16_t s = (int16_t)rd16(m, 0x00027E40u + 2u * ang);
    int32_t vx = (int32_t)speed * (int32_t)c;
    int32_t vy = (int32_t)speed * (int32_t)s;

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (uint32_t)vx);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (uint32_t)vy);
    r->d[1] = (uint32_t)vx;
    r->d[2] = (uint32_t)vy;
    r->a[3] = 0x00027E40u;

    wr32(m, F3_A5 + 0x30D2u, rd32(m, obj + 0x14u));
    wr32(m, F3_A5 + 0x30D6u, rd32(m, obj + 0x18u));
}

int Fn_121C86(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int8_t)rd8(m, F3_A5 + 0x326Eu) >= 6) {            /* 0x121C86..0x121C8C */
        Sound_PlayHooked(m, 0x0013u);                       /* 0x121D54..0x121D64 */
        wr32(m, F3_A5 + 0x30EEu, 0u);                       /* 0x121D68 */
        r->d[0] = setw(r->d[0], 0x003Cu);                   /* 0x121D6C */
        sub_0CBEB0(m, r);                                   /* 0x121D70 */
        uint16_t d0w = (uint16_t)(-((int16_t)((uint16_t)r->d[0] - 0x0078u))); /* 0x121D76..0x121D7A */
        r->d[0] = setw(r->d[0], d0w);
        wr16(m, obj + 0x06u, d0w);                          /* 0x121D7C */

        uint32_t rng = Rng_NextMem(m);                      /* 0x121D80 */
        d0w = (uint16_t)((rng & 0x001Fu) + 0x00CDu);        /* 0x121D86..0x121D8A */
        wr16(m, obj + 0x3Au, d0w);                          /* 0x121D8E */

        rng = Rng_NextMem(m);                               /* 0x121D92 */
        d0w = (uint16_t)((rng & 0x007Fu) + 0x0040u);        /* 0x121D98..0x121D9C */
        r->d[0] = (rng & 0xFFFF0000u) | d0w;
        wr16(m, obj + 0x3Cu, d0w);                          /* 0x121DA0 */

        wr32(m, obj, 0x00121DACu);                          /* 0x121DA4 */
        return 0;
    }

    sub_11B262(m, r);                                       /* 0x121C90 */
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + 0x0080u)); /* 0x121C96 */

    int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u);     /* 0x121C9C */
    wr16(m, obj + 0x06u, (uint16_t)cnt);
    if (cnt > 0) {                                          /* 0x121CA0 */
        wr32(m, obj, 0x00121C86u);                          /* 0x121C7E */
        return 0;
    }

    uint8_t b = rd8(m, F3_A5 + 0x30EEu);
    b &= (uint8_t)~0x08u;                                   /* 0x121CA2: bfclr {4:1} */
    b |= 0x40u;                                             /* 0x121CA8: bfset {1:1} */
    wr8(m, F3_A5 + 0x30EEu, b);
    wr8(m, F3_A5 + 0x30F2u, 0x22u);                         /* 0x121CAE */
    wr16(m, obj + 0x06u, 0x00B4u);                          /* 0x121CB4 */
    wr32(m, obj, 0x00121CC2u);                              /* 0x121CBA */
    return 0;
}
