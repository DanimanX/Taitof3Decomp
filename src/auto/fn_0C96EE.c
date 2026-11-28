/* ROM 0x0C96EE: turret/gun object tick (A0 = object).
 * Steps anim table 0x0C97D6 (0x0CBD52), copies (+0x14,+0x18) from the parent
 * at +0x08 plus (0x13,0x11), eases (+0x14,+0x18) by the sine/cosine pair at
 * +0x10 scaled by (+0x1E - 0x100) >> 5, draws the sprite (0x001282),
 * integrates (+0x2C,+0x30) by (+0x2E,+0x32), decays +0x22 by 0x2A0; while
 * (byte)+0x22 <= 0xC0 re-arms 0x0C96EE, else clears bits 1/2 of the parent's
 * +0x65 and moves on to 0x0C9752. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
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
        r->d[1] = (r->d[1] & 0xFFFF0000u) | d1w;
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* 68k memory bitfield {offset:width} with the offset counted from the MSB of
 * the byte at addr: fetch the big-endian 32-bit window and shift it down. */
static int32_t bfext_mem_s(const f3mem_t *m, uint32_t addr, unsigned off, unsigned w)
{
    uint32_t v = rd32(m, addr);
    v <<= off;
    v >>= (32u - w);
    if (w < 32u && (v & (1u << (w - 1u))) != 0u)
        v |= ~((1u << w) - 1u);          /* sign-extend the field */
    return (int32_t)v;
}

/* ROM 0x0C978E: place +0x22 into (+0x1E,+0x1F), add the 10-bit signed
 * (+0x2C,+0x30) velocities to (+0x14,+0x18), then add the two 16-bit
 * sine/cosine words at +0x10 scaled by ((+0x1E) - 0x100) >> 5. */
static void sub_0C978E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr8(m, obj + 0x1Fu, rd8(m, obj + 0x22u));             /* 0x0C978E */
    wr8(m, obj + 0x1Eu, rd8(m, obj + 0x22u));             /* 0x0C9794 */

    int32_t v14 = bfext_mem_s(m, obj + 0x2Cu, 0u, 10u);   /* 0x0C979A bfexts */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)v14));
    int32_t v18 = bfext_mem_s(m, obj + 0x30u, 0u, 10u);   /* 0x0C97A4 bfexts */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)v18));

    uint32_t d0 = (uint32_t)(uint8_t)rd8(m, obj + 0x1Eu); /* 0x0C97AE..0x0C97B4 */
    d0 = (d0 - 0x100u) & 0xFFFFu;
    uint32_t d1 = d0;                                     /* 0x0C97B8 */
    uint32_t a1 = rd32(m, obj + 0x10u);                   /* 0x0C97BA */

    int32_t s0 = (int32_t)(int16_t)(uint16_t)d0 * (int32_t)(int16_t)rd16(m, a1);
    d0 = (uint32_t)(s0 >> 5);                             /* 0x0C97BE..0x0C97C0 */
    a1 += 2u;
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d0));

    int32_t s1 = (int32_t)(int16_t)(uint16_t)d1 * (int32_t)(int16_t)rd16(m, a1);
    d1 = (uint32_t)(s1 >> 5);                             /* 0x0C97C6..0x0C97C8 */
    a1 += 2u;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d1));

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;

    draw_sprite_regs(m, r);                               /* 0x0C97CE -> 0x001282 */
}

int Fn_0C96EE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_Animate(m, obj, 0x000C97D6u);                        /* 0x0C96EE..0x0C96F2 -> 0x0CBD52 */

    uint32_t a1 = rd32(m, obj + 0x08u);                      /* 0x0C96F8 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0013u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0011u));

    sub_0C978E(m, r);                                        /* 0x0C9714 bsr */

    uint32_t d0 = setw(r->d[0], rd16(m, obj + 0x2Eu));       /* 0x0C9718 */
    wr16(m, obj + 0x2Cu, (uint16_t)(rd16(m, obj + 0x2Cu) + (uint16_t)d0));
    wr16(m, obj + 0x2Eu, (uint16_t)(rd16(m, obj + 0x2Eu) + 6u));
    d0 = setw(d0, rd16(m, obj + 0x32u));                     /* 0x0C9724 */
    wr16(m, obj + 0x30u, (uint16_t)(rd16(m, obj + 0x30u) + (uint16_t)d0));
    r->d[0] = d0;

    wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) + 0x02A0u)); /* 0x0C972C */
    if ((uint8_t)rd8(m, obj + 0x22u) <= 0xC0u) {             /* 0x0C9732 bls 0x0C96E6 */
        wr32(m, obj, 0x000C96EEu);
        return 0;
    }

    a1 = rd32(m, obj + 0x08u);                               /* 0x0C973A */
    wr8(m, a1 + 0x65u, (uint8_t)(rd8(m, a1 + 0x65u) & (uint8_t)~0x02u));
    wr8(m, a1 + 0x65u, (uint8_t)(rd8(m, a1 + 0x65u) & (uint8_t)~0x04u));
    wr32(m, obj, 0x000C9752u);                               /* 0x0C974A */
    return 0;
}
