/* ROM 0x13AE20: boss/enemy charge-glow intro tick (A0 = object).
 * Frees self if bit 4 of A5-0x144B is set or A5+0x32D6 >= 3. Draws two
 * layered sprites (0x3B7F0 at (-8,-8) with +0x1E=0, then 0x3B7F8 at
 * (-+0x22.b, -+0x22.b)), shrinks +0x22 by 0x40, ramps +0x1E by +0x0808
 * until 16-bit carry out, then clears +0x1E/+0x3A/+0x3E, sets (-0x18,-0x18)
 * offset, and transitions to 0x13AEB2. */
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

/* ROM 0x11B09A with exact D0/D1/A1 register tracking. */
static void sub_11B09A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t x = (int16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x54u) + rd16(m, F3_A5 + 0x30D2u));
    wr16(m, obj + 0x14u, (uint16_t)x);
    int16_t y = (int16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x56u) + rd16(m, F3_A5 + 0x30D6u));
    wr16(m, obj + 0x18u, (uint16_t)y);

    if (y < 0x00E8 && x < 0x0140) {
        uint32_t a1 = rd32(m, obj + 0x10u);
        int16_t h = (int16_t)((int16_t)rd16(m, a1) << 4);
        a1 += 2u;
        r->a[1] = a1;
        if ((int16_t)(h + y) > 0) {
            int16_t w = (int16_t)((int16_t)rd16(m, a1) << 4);
            a1 += 2u;
            r->a[1] = a1;
            if ((int16_t)(w + x) > 0)
                draw_sprite_regs(m, r);
        }
    }

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, obj + 0x54u)));
    uint16_t d0w = rd16(m, obj + 0x56u);
    r->d[0] = setw(r->d[0], d0w);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d0w));
}

int Fn_13AE20(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {  /* 0x13AE20..0x13AE26 */
        Obj_Free(m, obj);                           /* 0x13B062 */
        return 0;
    }
    if ((int16_t)rd16(m, F3_A5 + 0x32D6u) >= 3) {   /* 0x13AE2A..0x13AE30 */
        Obj_Free(m, obj);                           /* 0x13B062 */
        return 0;
    }

    wr8(m, obj + 0x1Du, 0xDEu);                     /* 0x13AE34 */
    wr32(m, obj + 0x10u, 0x0003B7F0u);              /* 0x13AE3A */
    uint16_t saved_1e = rd16(m, obj + 0x1Eu);       /* 0x13AE42 */
    wr16(m, obj + 0x1Eu, 0u);                       /* 0x13AE46 */
    wr16(m, obj + 0x54u, 0xFFF8u);                  /* 0x13AE4A */
    wr16(m, obj + 0x56u, 0xFFF8u);                  /* 0x13AE50 */
    sub_11B09A(m, r);                               /* 0x13AE56 */
    wr16(m, obj + 0x1Eu, saved_1e);                 /* 0x13AE5C */

    wr8(m, obj + 0x1Du, 0xC8u);                     /* 0x13AE60 */
    wr32(m, obj + 0x10u, 0x0003B7F8u);              /* 0x13AE66 */
    uint16_t off = (uint16_t)(int16_t)(int8_t)(0 - (int8_t)rd8(m, obj + 0x22u)); /* 0x13AE6E..0x13AE74 */
    r->d[0] = setw(r->d[0], off);
    wr16(m, obj + 0x54u, off);                      /* 0x13AE76 */
    wr16(m, obj + 0x56u, off);                      /* 0x13AE7A */
    sub_11B09A(m, r);                               /* 0x13AE7E */

    wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) - 0x0040u)); /* 0x13AE84 */
    uint32_t sum = (uint32_t)rd16(m, obj + 0x1Eu) + 0x0808u; /* 0x13AE8A */
    wr16(m, obj + 0x1Eu, (uint16_t)sum);
    if (sum <= 0xFFFFu) {                           /* 0x13AE90 bcc -> 0x13AE18 */
        wr32(m, obj, 0x0013AE20u);
        return 0;
    }

    wr16(m, obj + 0x1Eu, 0u);                       /* 0x13AE92 */
    wr16(m, obj + 0x3Au, 0u);                       /* 0x13AE96 */
    wr16(m, obj + 0x3Eu, 0u);                       /* 0x13AE9A */
    wr16(m, obj + 0x54u, 0xFFE8u);                  /* 0x13AE9E */
    wr16(m, obj + 0x56u, 0xFFE8u);                  /* 0x13AEA4 */
    wr32(m, obj, 0x0013AEB2u);                      /* 0x13AEAA */
    return 0;                                       /* 0x13AEB0 */
}
