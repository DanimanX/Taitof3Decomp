/* ROM 0x19AE0E: boss articulated sub-part tick (A0 = object, A0+0x08 = parent).
 * When A5+0x4B3A bit 7 (MSB) is set and +0x3A < 0x19C19C, switches to the
 * 0x19C19C animation sequence and runs 0x179A5C[idx].init. Then positions
 * (+0x50/+0x54, +0x14/+0x18) from parent (+0x50,+0x54) + (+0x16,+0x1A), steps
 * 0x179A5C[+0x40].tick, offsets (+0x14,+0x18) by sprite half-size*8, and
 * either transitions to 0x17A85C (with +0x06=2) if sprite == 0x28740, or
 * draws sprite (0x001282) and re-arms 0x19AE0E. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
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
    uint32_t hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);

    if (hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = setw(r->d[1], d1w);
    } else {
        uint16_t cols = (uint16_t)(hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

int Fn_19AE0E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 + 0x4B3Au) & 0x80u) != 0u) {       /* 0x19AE0E bftst {0:1} */
        if ((int32_t)rd32(m, obj + 0x3Au) < (int32_t)0x0019C19Cu) { /* 0x19AE18 */
            uint32_t a1 = 0x0019C19Cu;
            uint16_t idx = rd16(m, a1);
            a1 += 2u;
            wr16(m, obj + 0x40u, idx);
            wr32(m, obj + 0x3Au, a1);
            if (Obj_AnimInit(m, obj, idx) < 0)
                return -1;
        }
    }

    uint32_t a4 = rd32(m, obj + 0x08u);                  /* 0x19AE40 */
    r->a[4] = a4;
    uint16_t x = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t y = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, x);                             /* 0x19AE54 */
    wr16(m, obj + 0x54u, y);                             /* 0x19AE58 */
    wr16(m, obj + 0x14u, x);                             /* 0x19AE5C */
    wr16(m, obj + 0x18u, y);                             /* 0x19AE60 */

    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)  /* 0x19AE64..0x19AE72 */
        return -1;

    uint32_t a1 = rd32(m, obj + 0x10u);                  /* 0x19AE74 */
    uint16_t sy = (uint16_t)(rd16(m, a1) << 3);          /* 0x19AE78, 0x19AE7E */
    uint16_t sx = (uint16_t)(rd16(m, a1 + 2u) << 3);     /* 0x19AE7A, 0x19AE7C */
    a1 += 4u;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - sx)); /* 0x19AE80 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - sy)); /* 0x19AE84 */

    if (rd32(m, obj + 0x10u) == 0x00028740u) {           /* 0x19AE88..0x19AE90 */
        r->d[0] = setw(r->d[0], sx);
        r->d[1] = setw(r->d[1], sy);
        r->a[1] = a1;
        wr16(m, obj + 0x06u, 0x0002u);                   /* 0x17A84C (0x17A852 beq never taken) */
        wr32(m, obj, 0x0017A85Cu);                       /* 0x17A854 */
        return 0;
    }

    draw_sprite_regs(m, r);                              /* 0x19AE96 */
    wr32(m, obj, 0x0019AE0Eu);                           /* 0x19AE9C -> 0x19AE06 */
    return 0;
}
