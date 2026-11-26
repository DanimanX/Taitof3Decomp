/* ROM 0x13ADD4: boss/enemy shrink-out / zoom-down transition tick (A0 = object).
 * Frees self via 0x0105EC if bit 4 of A5-0x144B is set or A5+0x32D6 >= 3.
 * Otherwise sets (+0x54, +0x56) to -(int8_t)+0x22, draws sprite via 0x11B09A,
 * advances +0x22 by 0x20, subtracts 0x404 from +0x1E and re-arms 0x13ADD4 while
 * unsigned >= 0x404; else resets +0x1E = 0, +0x22 = 0x800, and transitions to
 * 0x13AE20. */
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
    uint16_t x = (uint16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x54u) + rd16(m, F3_A5 + 0x30D2u));
    wr16(m, obj + 0x14u, x);
    uint16_t y = (uint16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x56u) + rd16(m, F3_A5 + 0x30D6u));
    wr16(m, obj + 0x18u, y);
    r->d[0] = setw(r->d[0], y);

    if ((int16_t)y < 0x00E8 && (int16_t)x < 0x0140) {
        uint32_t a1 = rd32(m, obj + 0x10u);
        int16_t h = (int16_t)(rd16(m, a1) << 4);
        a1 += 2u;
        r->d[0] = setw(r->d[0], (uint16_t)(h + (int16_t)y));
        if ((int16_t)r->d[0] > 0) {
            int16_t w = (int16_t)(rd16(m, a1) << 4);
            a1 += 2u;
            r->d[0] = setw(r->d[0], (uint16_t)(w + (int16_t)x));
            if ((int16_t)r->d[0] > 0) {
                r->a[1] = a1;
                draw_sprite_regs(m, r);
            } else {
                r->a[1] = a1;
            }
        } else {
            r->a[1] = a1;
        }
    }

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, obj + 0x54u)));
    uint16_t offy = rd16(m, obj + 0x56u);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - offy));
    r->d[0] = setw(r->d[0], offy);
}

int Fn_13ADD4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u
        || (int16_t)rd16(m, F3_A5 + 0x32D6u) >= 3) {    /* 0x13ADD4..0x13ADE4 */
        Obj_Free(m, obj);                               /* 0x13B062 */
        return 0;
    }

    uint16_t off = (uint16_t)(int16_t)(int8_t)(0 - (int8_t)rd8(m, obj + 0x22u)); /* 0x13ADE8..0x13ADEE */
    r->d[0] = setw(r->d[0], off);
    wr16(m, obj + 0x54u, off);                          /* 0x13ADF0 */
    wr16(m, obj + 0x56u, off);                          /* 0x13ADF4 */
    sub_11B09A(m, r);                                   /* 0x13ADF8 */

    wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) + 0x0020u)); /* 0x13ADFE */

    uint16_t old_1e = rd16(m, obj + 0x1Eu);             /* 0x13AE04 */
    wr16(m, obj + 0x1Eu, (uint16_t)(old_1e - 0x0404u));
    if (old_1e >= 0x0404u) {                            /* 0x13AE0A bcc */
        wr32(m, obj, 0x0013ADD4u);                      /* 0x13ADCC */
        return 0;
    }

    wr16(m, obj + 0x1Eu, 0u);                           /* 0x13AE0C */
    wr16(m, obj + 0x22u, 0x0800u);                      /* 0x13AE12 */
    wr32(m, obj, 0x0013AE20u);                          /* 0x13AE18 */
    return 0;
}
