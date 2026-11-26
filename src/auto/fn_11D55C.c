/* ROM 0x11D55C: boss/parent-linked sprite part tick (A0 = object).
 * Frees self (0x0105EC) if parent (+0x08)->+0x59 != 0. Otherwise selects
 * sprite pointer from table (+0x0C)[A5+0x3256], registers collision and
 * updates flash (0x11B06E), inherits/looks up palette +0x1D (0x11DECC),
 * draws at screen pos (0x11B09A), and re-arms 0x11D55C. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
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

/* ROM 0x11DECC: inherit parent+0x1D or look up table 0x11DE76 / 0x11DE8E. */
static void sub_11DECC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, obj + 0x08u);                            /* 0x11DECC */
    r->a[1] = a1;
    uint8_t c = rd8(m, a1 + 0x1Du);                                /* 0x11DED0 */
    wr8(m, obj + 0x1Du, c);
    if (c != 0u)                                                   /* 0x11DED6 */
        return;

    a1 = 0x0011DE76u;                                              /* 0x11DEDA */
    int16_t d0w = (int16_t)rd16(m, obj + 0x40u);                   /* 0x11DEDE */
    if (d0w >= 0x0C) {                                             /* 0x11DEE2..0x11DEE6 */
        a1 = 0x0011DE8Eu;                                          /* 0x11DEEA */
        d0w = 0;                                                   /* 0x11DEEE */
    }
    d0w = (int16_t)(d0w + (int16_t)rd16(m, F3_A5 + 0x3258u));     /* 0x11DEF0 */
    wr8(m, obj + 0x1Du, rd8(m, a1 + (uint32_t)(int32_t)d0w));     /* 0x11DEF4 */
    r->d[0] = setw(r->d[0], (uint16_t)d0w);
    r->a[1] = a1;
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

int Fn_11D55C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, obj + 0x08u);                            /* 0x11D55C */
    r->a[1] = a1;

    if (rd8(m, a1 + 0x59u) != 0u) {                                /* 0x11D560..0x11D564 */
        Obj_Free(m, obj);                                          /* 0x11D588..0x11D590 (0x0105EC) */
        return 0;
    }

    a1 = rd32(m, obj + 0x0Cu);                                     /* 0x11D568 */
    r->a[1] = a1;
    uint16_t idx = rd16(m, F3_A5 + 0x3256u);                       /* 0x11D56C */
    r->d[0] = setw(r->d[0], idx);
    wr32(m, obj + 0x10u, rd32(m, a1 + (uint32_t)((int32_t)(int16_t)idx * 4))); /* 0x11D570 */

    Obj_11B06E(m, obj);                                            /* 0x11D576 */
    if (rd8(m, F3_A5 + 0x30F3u) == 0u && rd32(m, obj + 0x2Cu) != 0u)
        r->a[1] = rd32(m, F3_A5 + 0x0906u);

    sub_11DECC(m, r);                                              /* 0x11D57C */
    sub_11B09A(m, r);                                              /* 0x11D580 */
    wr32(m, obj, 0x0011D55Cu);                                     /* 0x11D586 -> 0x11D554 */
    return 0;
}
