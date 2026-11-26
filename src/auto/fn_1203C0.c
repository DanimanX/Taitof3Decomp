/* ROM 0x1203C0: parent-linked part idle/trigger tick (A0 = object).
 * Frees self (0x0105EC) if parent (+0x08)->+0x59 != 0. If A5-0x144B bit 4 is
 * set and (Rng_NextMem & 0x7F) == 0, sets +0x42 = -0x20, clears +0x1D, and
 * transitions to 0x1203FE. Otherwise draws via 0x11B09A and re-arms 0x1203C0. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);

    if (spr_hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = setw(r->d[1], d1w);
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x11B09A with D0/D1/A1 register tracking. */
static void sub_11B09A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t x = (int16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x54u) + rd16(m, F3_A5 + 0x30D2u));
    wr16(m, obj + 0x14u, (uint16_t)x);                             /* 0x11B09A..0x11B0A6 */
    int16_t y = (int16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x56u) + rd16(m, F3_A5 + 0x30D6u));
    wr16(m, obj + 0x18u, (uint16_t)y);                             /* 0x11B0AA..0x11B0B6 */

    if (y < 0x00E8 && x < 0x0140) {                                /* 0x11B0BA..0x11B0CA */
        uint32_t a1 = rd32(m, obj + 0x10u);                        /* 0x11B0CE */
        int16_t h = (int16_t)((int16_t)rd16(m, a1) << 4);          /* 0x11B0D2..0x11B0D4 */
        a1 += 2u;
        r->a[1] = a1;
        if ((int16_t)(h + y) > 0) {                                /* 0x11B0D6..0x11B0DA */
            int16_t w = (int16_t)((int16_t)rd16(m, a1) << 4);      /* 0x11B0DE..0x11B0E0 */
            a1 += 2u;
            r->a[1] = a1;
            if ((int16_t)(w + x) > 0)                              /* 0x11B0E2..0x11B0E6 */
                draw_sprite_regs(m, r);                            /* 0x11B0EA */
        }
    }

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, obj + 0x54u))); /* 0x11B0F0..0x11B0F4 */
    uint16_t d0w = rd16(m, obj + 0x56u);                           /* 0x11B0F8 */
    r->d[0] = setw(r->d[0], d0w);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d0w)); /* 0x11B0FC */
}

int Fn_1203C0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, obj + 0x08u);                            /* 0x1203C0 */
    r->a[1] = a1;

    if (rd8(m, a1 + 0x59u) != 0u) {                                /* 0x1203C4..0x1203C8 */
        Obj_Free(m, obj);                                          /* 0x120458..0x120460 (0x0105EC) */
        return 0;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {                 /* 0x1203CC..0x1203D2 */
        uint32_t rng = Rng_NextMem(m);                             /* 0x1203D6 (0x001D8A) */
        uint16_t d0w = (uint16_t)(rng & 0x007Fu);                  /* 0x1203DC */
        r->d[0] = (rng & 0xFFFF0000u) | d0w;
        if (d0w == 0u) {                                           /* 0x1203E0 */
            wr16(m, obj + 0x42u, 0xFFE0u);                         /* 0x1203EC */
            wr8(m, obj + 0x1Du, 0u);                               /* 0x1203F2 */
            wr32(m, obj, 0x001203FEu);                             /* 0x1203F6 */
            return 0;
        }
    }

    sub_11B09A(m, r);                                              /* 0x1203E4 */
    wr32(m, obj, 0x001203C0u);                                     /* 0x1203EA -> 0x1203B8 */
    return 0;
}
