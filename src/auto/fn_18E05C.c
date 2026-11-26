/* ROM 0x18E05C: timed boss/enemy projectile burst emitter tick (A0 = object).
 * Counts down +0x06; if > 0 re-arms 0x18E05C. Otherwise syncs position from
 * parent (+0x08) + (+0x16, +0x1A), spawns projectile (+0x0C) with angle +0x20
 * (group 9, 2), plays hooked sound 0x3C, advances angle +0x20 by +0x60 * +0x42,
 * subtracts +0x42 from remaining angle span +0x62, and either reloads +0x06 from
 * +0x42 and re-arms 0x18E05C (while +0x62 > 0 and A5+0x4B38 != 0) or sets
 * +0x06 = 2 and transitions to 0x18E196. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_pool.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_18E05C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x18E05C */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                              /* 0x18E060 */
        wr32(m, obj, 0x0018E05Cu);                       /* 0x18E054 */
        return 0;
    }

    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    for (;;) {
        uint32_t parent = rd32(m, obj + 0x08u);          /* 0x18E062 */
        uint16_t x = (uint16_t)(rd16(m, parent + 0x50u) + rd16(m, obj + 0x16u)); /* 0x18E066..0x18E06E */
        uint16_t y = (uint16_t)(rd16(m, parent + 0x54u) + rd16(m, obj + 0x1Au)); /* 0x18E06A..0x18E072 */
        wr16(m, obj + 0x50u, x);                         /* 0x18E076 */
        wr16(m, obj + 0x54u, y);                         /* 0x18E07A */
        wr16(m, obj + 0x14u, x);                         /* 0x18E07E */
        wr16(m, obj + 0x18u, y);                         /* 0x18E082 */

        d1 = (uint32_t)rd8(m, obj + 0x20u);              /* 0x18E086..0x18E088 */
        d0 = Obj_Spawn(m, obj, rd32(m, obj + 0x0Cu), 9u, 2u, d1); /* 0x18E08C..0x18E0A0 */
        Sound_PlayHooked(m, 0x003Cu);                    /* 0x18E0A4..0x18E0B4 */

        uint16_t step = rd16(m, obj + 0x42u);
        int32_t prod = (int32_t)(int16_t)rd16(m, obj + 0x60u) * (int32_t)(int16_t)step; /* 0x18E0B8..0x18E0BC */
        wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + (uint16_t)prod)); /* 0x18E0C0 */
        d0 = setw((uint32_t)prod, step);                 /* 0x18E0C4 */
        int16_t rem = (int16_t)(rd16(m, obj + 0x62u) - step); /* 0x18E0C8 */
        wr16(m, obj + 0x62u, (uint16_t)rem);
        if (rem <= 0)                                    /* 0x18E0CC */
            break;

        if (rd16(m, F3_A5 + 0x4B38u) == 0u)              /* 0x18E042..0x18E048 */
            break;
        wr16(m, obj + 0x06u, step);                      /* 0x18E04C */
        if (step != 0u) {                                /* 0x18E052 */
            wr32(m, obj, 0x0018E05Cu);                   /* 0x18E054 */
            r->d[0] = d0;
            r->d[1] = d1;
            return 0;
        }
    }

    wr16(m, obj + 0x06u, 2u);                            /* 0x18E186 */
    wr32(m, obj, 0x0018E196u);                           /* 0x18E18E */
    r->d[0] = d0;
    r->d[1] = d1;
    return 0;
}
