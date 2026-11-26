/* ROM 0x11FB0A: boss sub-part/turret tick (A0 = object, A0+0x08 = parent).
 * Frees itself if parent+0x59 != 0. Otherwise indexes pose/hitbox record from
 * (+0x0C)[A5+0x3262] and unpacks via 0x12049C, optionally fires 0x11B5E6 shot
 * (arg from 0x11FB98[0..7]) when A5+0x30EE bit 3 from MSB is set and
 * 0x0CC2BC(0x0A) == 0, pushes collision lists & updates flash (0x11B06E),
 * copies parent+0x1D to +0x1D, draws sprite (0x11B09A), and re-arms 0x11FB0A. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w) { return (reg & 0xFFFF0000u) | w; }
static uint32_t swap32(uint32_t v) { return (v << 16) | (v >> 16); }

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

/* ROM 0x0CC2BC: D0 = (quot << 16) | rem of (A5+0x878) / max(1, ((0x180 - A5+0x87E)*d0) >> 8). */
static void sub_0CC2BC(const f3mem_t *m, uint16_t d0w, uint32_t *d0_out, uint32_t *d1_out)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = ((uint32_t)d1w * (uint32_t)d0w) << 8;
    d1 = swap32(d1);
    if ((uint16_t)d1 == 0u)
        d1 = (d1 & 0xFFFF0000u) | 1u;
    uint16_t div = (uint16_t)d1;
    uint32_t num = rd16(m, F3_A5 + 0x0878u);
    uint16_t quot = (uint16_t)(num / div);
    uint16_t rem = (uint16_t)(num % div);
    *d0_out = ((uint32_t)quot << 16) | rem;
    *d1_out = d1;
}

/* ROM 0x12049C: unpack sprite ptr and 4 signed-byte hitbox bounds. */
static void sub_12049C(const f3mem_t *m, uint32_t obj, uint32_t a1, uint32_t *d0_out)
{
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x30u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x2Eu, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    int32_t last = (int32_t)(int8_t)rd8(m, a1);
    wr16(m, obj + 0x32u, (uint16_t)last);
    *d0_out = (uint32_t)last;
}

/* ROM 0x11B09A with exact D0/D1/A1 register tracking. */
static void sub_11B09A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t x = (int16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x54u) + rd16(m, F3_A5 + 0x30D2u));
    wr16(m, obj + 0x14u, (uint16_t)x);
    int16_t y = (int16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x56u) + rd16(m, F3_A5 + 0x30D6u));
    wr16(m, obj + 0x18u, (uint16_t)y);
    r->d[0] = setw(r->d[0], (uint16_t)y);

    if (y < 0x00E8 && x < 0x0140) {
        uint32_t a1 = rd32(m, obj + 0x10u);
        int16_t h = (int16_t)(rd16(m, a1) << 4);
        a1 += 2u;
        r->a[1] = a1;
        r->d[0] = setw(r->d[0], (uint16_t)(h + y));
        if ((int16_t)r->d[0] > 0) {
            int16_t w = (int16_t)(rd16(m, a1) << 4);
            a1 += 2u;
            r->a[1] = a1;
            r->d[0] = setw(r->d[0], (uint16_t)(w + x));
            if ((int16_t)r->d[0] > 0)
                draw_sprite_regs(m, r);
        }
    }

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, obj + 0x54u)));
    uint16_t offy = rd16(m, obj + 0x56u);
    r->d[0] = setw(r->d[0], offy);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - offy));
}

int Fn_11FB0A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t parent = rd32(m, obj + 0x08u);              /* 0x11FB0A */
    r->a[1] = parent;

    if (rd8(m, parent + 0x59u) != 0u) {                  /* 0x11FB0E..0x11FB12 */
        Obj_Free(m, obj);                                /* 0x11FB8A..0x11FB96 */
        return 0;
    }

    uint32_t d0 = (uint32_t)rd8(m, F3_A5 + 0x3262u);     /* 0x11FB1A..0x11FB1C */
    uint32_t a1 = rd32(m, rd32(m, obj + 0x0Cu) + d0 * 4u); /* 0x11FB16..0x11FB20 */
    sub_12049C(m, obj, a1, &d0);                         /* 0x11FB24 */

    uint32_t d1 = r->d[1];
    /* 0x11FB28: bftst (0x30ee,A5){3:1} -> bit 3 from MSB = 0x10 */
    if ((rd8(m, F3_A5 + 0x30EEu) & 0x10u) != 0u) {
        sub_0CC2BC(m, 0x000Au, &d0, &d1);                /* 0x11FB32..0x11FB36 */
        if ((uint16_t)d0 == 0u) {                        /* 0x11FB3C */
            d0 = swap32(d0);                             /* 0x11FB42 */
            d0 = setw(d0, (uint16_t)(d0 & 0x000Fu));     /* 0x11FB44 */
            d0 &= ~0x00000008u;                          /* 0x11FB48 bclr #3,D0 */
            d1 = rd32(m, 0x0011FB98u + (uint32_t)(uint16_t)d0 * 4u); /* 0x11FB4C..0x11FB52 */
            d0 = Obj_Spawn(m, obj, 0x0011B5E6u, 9u, 2u, d1); /* 0x11FB56..0x11FB6C */
        }
    }

    Obj_11B06E(m, obj);                                  /* 0x11FB70 */
    a1 = rd32(m, obj + 0x08u);                           /* 0x11FB76 */
    wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));             /* 0x11FB7A */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;

    sub_11B09A(m, r);                                    /* 0x11FB80 */
    wr32(m, obj, 0x0011FB0Au);                           /* 0x11FB86 -> 0x11FB02 */
    return 0;
}
