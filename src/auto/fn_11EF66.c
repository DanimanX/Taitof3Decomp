/* ROM 0x11EF66: boss camera/screen-bob wave phase (A0 = object).
 * Calls 0x11F488 to integrate sin(obj+0x20.b) * obj+0x42 into camera Y
 * (A5+0x30D6) via delta against +0x18. If A5+0x1D58 bit 1 is set, nudges
 * camera X (A5+0x30D2) toward 0xA0 by +/-0x8000. Advances +0x20 by +0x400 and
 * re-arms 0x11EF66 while <= 0. Otherwise plays hooked sound 0x25, decrements
 * +0x06, resets +0x20=0x8000, +0x42=0x80, +0x18=0 and re-arms 0x11EF66 while
 * +0x06 > 0, else clears A5+0x30EE bits {4,12,13}, sets +0x06=0x3C, and
 * transitions to 0x11EFD2. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

/* ROM 0x11F488: step camera Y (A5+0x30D6) by delta of sin(+0x20.b)*(+0x42). */
static void sub_11F488_regs(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    uint32_t a1 = 0x00027E40u;                                 /* 0x11F488 */
    r->a[1] = a1;
    uint32_t idx = rd8(m, obj + 0x20u);                        /* 0x11F48E..0x11F490 */
    int16_t s = (int16_t)rd16(m, a1 + idx * 2u);               /* 0x11F494 */
    int32_t d1 = (int32_t)s * (int32_t)(int16_t)rd16(m, obj + 0x42u); /* 0x11F49A */
    uint32_t d0 = (uint32_t)d1 - rd32(m, obj + 0x18u);         /* 0x11F49E..0x11F4A0 */
    wr32(m, F3_A5 + 0x30D6u, rd32(m, F3_A5 + 0x30D6u) + d0);   /* 0x11F4A4 */
    wr32(m, obj + 0x18u, (uint32_t)d1);                        /* 0x11F4A8 */
    r->d[0] = d0;
    r->d[1] = (uint32_t)d1;
}

int Fn_11EF66(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    sub_11F488_regs(m, r, obj);                                /* 0x11EF66 */

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {             /* 0x11EF6A..0x11EF70 */
        if ((int16_t)rd16(m, F3_A5 + 0x30D2u) > 0x00A0)        /* 0x11EF72..0x11EF78 */
            wr32(m, F3_A5 + 0x30D2u, rd32(m, F3_A5 + 0x30D2u) - 0x00008000u); /* 0x11EF7A */
        else
            wr32(m, F3_A5 + 0x30D2u, rd32(m, F3_A5 + 0x30D2u) + 0x00008000u); /* 0x11EF84 */
    }

    int16_t ang = (int16_t)(rd16(m, obj + 0x20u) + 0x0400u);   /* 0x11EF8C */
    wr16(m, obj + 0x20u, (uint16_t)ang);
    if (ang <= 0) {                                            /* 0x11EF92..0x11EF96 */
        wr32(m, obj + 0x00u, 0x0011EF66u);                     /* 0x11EF5E */
        return 0;
    }

    Sound_PlayHooked(m, 0x0025u);                              /* 0x11EF98..0x11EFA8 */
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);          /* 0x11EFAC */
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {                                               /* 0x11EFB0 */
        wr16(m, obj + 0x20u, 0x8000u);                         /* 0x11EF4E */
        wr16(m, obj + 0x42u, 0x0080u);                         /* 0x11EF54 */
        wr32(m, obj + 0x18u, 0u);                              /* 0x11EF5A */
        wr32(m, obj + 0x00u, 0x0011EF66u);                     /* 0x11EF5E */
        return 0;
    }

    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) & ~0x08u)); /* 0x11EFB2 {12:1} */
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & ~0x08u)); /* 0x11EFB8 {4:1} */
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) & ~0x04u)); /* 0x11EFBE {13:1} */
    wr16(m, obj + 0x06u, 0x003Cu);                             /* 0x11EFC4 */
    wr32(m, obj + 0x00u, 0x0011EFD2u);                         /* 0x11EFCA */
    return 0;
}
