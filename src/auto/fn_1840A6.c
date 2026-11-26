/* ROM 0x1840A6: periodic radial/fan boss shot spawner tick (A0 = object).
 * If A5+0x4B38 == 0, sets +0x64=0x10 and advances to 0x184122. Otherwise
 * tests (+0x42.b % +0x5C.w) == 0; when 0, copies parent (+0x08)->(+0x50/+0x54)
 * + (+0x16/+0x1A) into (+0x50/+0x54) and (+0x14/+0x18) and spawns +0x0C with
 * arg +0x20.b (group 9, 2 groups). Advances +0x20.b += +0x43.b, decrements
 * +0x42.b, re-arming 0x1840A6 while (s8)+0x42 > 0 or advancing to 0x184122. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_1840A6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if (rd16(m, F3_A5 + 0x4B38u) == 0u) {                     /* 0x1840A6..0x1840AC */
        wr16(m, obj + 0x64u, 0x0010u);                        /* 0x184114 */
        wr32(m, obj, 0x00184122u);                            /* 0x18411A */
        return 0;
    }

    uint16_t d1w = rd16(m, obj + 0x5Cu);                      /* 0x1840B0 */
    r->d[1] = setw(r->d[1], d1w);
    uint32_t d0 = (uint32_t)rd8(m, obj + 0x42u);              /* 0x1840B4 */
    if (d1w != 0u) {                                          /* 0x1840BA */
        uint32_t q = d0 / d1w;
        uint32_t rem = d0 % d1w;
        if (q <= 0xFFFFu)
            d0 = (rem << 16) | q;
    }
    d0 = (d0 << 16) | (d0 >> 16);                             /* 0x1840BC swap D0 */
    r->d[0] = d0;

    if ((uint16_t)d0 == 0u) {                                 /* 0x1840BE..0x1840C0 */
        uint32_t a4 = rd32(m, obj + 0x08u);                   /* 0x1840C4 */
        r->a[4] = a4;
        uint16_t x = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u)); /* 0x1840C8..0x1840D0 */
        uint16_t y = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au)); /* 0x1840CC..0x1840D4 */
        wr16(m, obj + 0x50u, x);                              /* 0x1840D8 */
        wr16(m, obj + 0x54u, y);                              /* 0x1840DC */
        wr16(m, obj + 0x14u, x);                              /* 0x1840E0 */
        wr16(m, obj + 0x18u, y);                              /* 0x1840E4 */
        uint32_t arg = (uint32_t)rd8(m, obj + 0x20u);         /* 0x1840E8 */
        r->d[1] = arg;
        d0 = Obj_Spawn(m, obj, rd32(m, obj + 0x0Cu), 9u, 2u, arg); /* 0x1840EE..0x184102 */
    }

    uint8_t step = rd8(m, obj + 0x43u);                       /* 0x184106 */
    r->d[0] = (d0 & 0xFFFFFF00u) | step;
    wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) + step)); /* 0x18410A */

    int8_t cnt = (int8_t)(rd8(m, obj + 0x42u) - 1u);          /* 0x18410E */
    wr8(m, obj + 0x42u, (uint8_t)cnt);
    if (cnt > 0) {                                            /* 0x184112 */
        wr32(m, obj, 0x001840A6u);                            /* 0x18409E */
        return 0;
    }

    wr16(m, obj + 0x64u, 0x0010u);                            /* 0x184114 */
    wr32(m, obj, 0x00184122u);                                /* 0x18411A */
    return 0;
}
