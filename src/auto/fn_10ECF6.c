/* ROM 0x10ECF6: enemy variant init (A0 = object).
 * Looks up {param_blk, frame_rec} pair at 0x10EF3A[+0x10 * 8], seeds enemy
 * stats (0x0CEDE4) and initial frame (0x0CFA86), unpacks +0x68 parameters into
 * +0x3A/+0x3E/+0x42/+0x3C/+0x40/+0x44, jitters speed (+0x60) and heading (+0x58)
 * modulo 360 via 0x0D0E3A, computes (+0x50, +0x54) velocity via 0x0D2D44,
 * sets +0x34 flags, subtracts scroll delta (A5+0x4224/4228), draws sprite
 * (0x001282), and installs tick 0x10EDA0. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_shared.h"
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

/* ROM 0x0D2D44: scale (0x0D2D64[angle], 0x0D2ECC[angle]) by speed (0..50). */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    *out_d0 = (uint32_t)(d0 * (uint32_t)d1w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d1w);
}

int Fn_10ECF6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t idx = (int16_t)rd16(m, obj + 0x10u);         /* 0x10ECF6 */
    uint32_t a3 = 0x0010EF3Au + (uint32_t)(int32_t)idx * 8u; /* 0x10ECFA */

    Obj_0CEDE4(m, obj, rd32(m, a3));                     /* 0x10ED02..0x10ED04 */
    Obj_SetFrame(m, obj, rd32(m, a3 + 4u));              /* 0x10ED0A..0x10ED0C */

    {
        uint32_t a1 = rd32(m, obj + 0x68u);              /* 0x10ED12 */
        wr16(m, obj + 0x3Au, rd16(m, a1)); a1 += 2u;     /* 0x10ED16 */
        wr16(m, obj + 0x3Eu, rd16(m, a1)); a1 += 2u;     /* 0x10ED1A */
        uint32_t d0 = rd32(m, a1); a1 += 4u;             /* 0x10ED1E */
        wr16(m, obj + 0x42u, (uint16_t)d0);              /* 0x10ED20 */
        wr16(m, obj + 0x3Cu, rd16(m, a1)); a1 += 2u;     /* 0x10ED24 */
        wr16(m, obj + 0x40u, rd16(m, a1)); a1 += 2u;     /* 0x10ED28 */
        wr32(m, obj + 0x44u, rd32(m, a1));               /* 0x10ED2C */
    }

    {
        uint16_t d1w = (uint16_t)((Obj_Next4Way(m) & 0x0003u) + rd16(m, obj + 0x60u)); /* 0x10ED30..0x10ED3E */
        int16_t ang = (int16_t)((Obj_Next4Way(m) & 0x000Fu) - 8 + (int16_t)rd16(m, obj + 0x58u)); /* 0x10ED40..0x10ED4C */
        if (ang < 0)
            ang = (int16_t)(ang + 0x0168);               /* 0x10ED64 */
        else if (ang >= 0x0168)
            ang = (int16_t)(ang - 0x0168);               /* 0x10ED5C */

        uint32_t d0, d1;
        sub_0D2D44(m, (uint16_t)ang, d1w, &d0, &d1);     /* 0x10ED68 */
        wr32(m, obj + 0x50u, d0);                        /* 0x10ED6E */
        wr32(m, obj + 0x54u, d1);                        /* 0x10ED72 */
        r->d[1] = d1;
    }

    wr8(m, obj + 0x34u, (uint8_t)((rd8(m, obj + 0x34u) & ~0x02u) | 0x04u)); /* 0x10ED76..0x10ED7C */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - rd32(m, F3_A5 + 0x4224u)); /* 0x10ED82..0x10ED86 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u)); /* 0x10ED8A..0x10ED8E */

    draw_sprite_regs(m, r);                              /* 0x10ED92 */
    wr32(m, obj, 0x0010EDA0u);                           /* 0x10ED98 */
    return 0;
}
