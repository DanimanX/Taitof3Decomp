/* ROM 0x196C40: boss main body tick & death transition (A0 = object).
 * When HP (+0x24) > 0: runs 0x196AE0 (oscillates/nudges +0x20 by +/-+0x60 when
 * hit +0x35 != 0, copies +0x50/+0x54 to +0x14/+0x18, ticks 0x179A5C[+0x40],
 * centers by sprite {w,h}*8, updates hit-flash +0x1D, clears +0x35, draws
 * sprite via 0x001282, pushes to collision lists A5+0x8FE/A5+0x906 if
 * A5+0x4B3B bit 0 == 0) and re-arms 0x196C40.
 * When HP <= 0: credits 0x7530 via 0x17B02E, adds 0x12C5F to A5+0x886, sets
 * A5+0x4B26 bit 5, copies +0x34/+0x36 to A5+0x4B20/+0x4B1C, sets +0x23 bit 2,
 * spawns 0x179D92 (group 5, ngroups 2), sets +0x06=2, and arms 0x196BE8. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
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

/* ROM 0x17B02E: resolve score receiver from +0x35 (+0x36->+8 if 1, +0x36 if 2, else -1). */
static uint32_t sub_17B02E_regs(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    uint8_t mode = rd8(m, obj + 0x35u);                        /* 0x17B02E */
    if (mode == 1u) {
        uint32_t a1 = rd32(m, obj + 0x36u);                    /* 0x17B038 */
        r->a[1] = a1;
        return rd32(m, a1 + 0x08u);                            /* 0x17B03C */
    }
    if (mode == 2u)                                            /* 0x17B042 */
        return rd32(m, obj + 0x36u);                           /* 0x17B04C */
    return 0xFFFFFFFFu;                                        /* 0x17B052 */
}

/* ROM 0x196AE0: anim tick, hit-flash, sprite draw, and collision push. */
static int sub_196AE0_regs(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) != 0u) {                           /* 0x196AE0 */
        uint8_t d0b = rd8(m, obj + 0x60u);                     /* 0x196AE8 */
        if ((int8_t)rd8(m, obj + 0x20u) > 0)                   /* 0x196AEC..0x196AF0 */
            d0b = (uint8_t)(0u - d0b);                         /* 0x196AF4 */
        wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) + d0b)); /* 0x196AF6 */
    }

    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));                /* 0x196AFA */
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));                /* 0x196B00 */

    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)        /* 0x196B06..0x196B14 */
        return -1;

    uint32_t a1 = rd32(m, obj + 0x10u);                        /* 0x196B16 */
    uint16_t sy = (uint16_t)(rd16(m, a1) << 3);                /* 0x196B1A, 0x196B20 */
    uint16_t sx = (uint16_t)(rd16(m, a1 + 2u) << 3);           /* 0x196B1C, 0x196B1E */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - sx)); /* 0x196B22 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - sy)); /* 0x196B26 */

    if (rd8(m, obj + 0x1Du) == 6u)                             /* 0x196B2A */
        wr8(m, obj + 0x1Du, 8u);                               /* 0x196B50 */
    else if (rd8(m, obj + 0x35u) == 0u)                        /* 0x196B34 */
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));          /* 0x196B3C */
    else
        wr8(m, obj + 0x1Du, 6u);                               /* 0x196B46 */

    wr8(m, obj + 0x35u, 0u);                                   /* 0x196B5A */
    draw_sprite_regs(m, r);                                    /* 0x196B5E */

    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x01u) == 0u) {             /* 0x196B64 bftst {15:1} */
        uint32_t p1 = rd32(m, F3_A5 + 0x08FEu);                /* 0x196B6E */
        wr32(m, p1, obj);
        wr32(m, F3_A5 + 0x08FEu, p1 + 4u);
        uint32_t p2 = rd32(m, F3_A5 + 0x0906u);                /* 0x196B78 */
        wr32(m, p2, obj);
        p2 += 4u;
        wr32(m, F3_A5 + 0x0906u, p2);
        r->a[1] = p2;
    }
    return 0;
}

int Fn_196C40(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {                  /* 0x196C40..0x196C44 */
        uint32_t d0 = sub_17B02E_regs(m, r, obj);              /* 0x196B84 */
        if ((int32_t)d0 >= 0) {                                /* 0x196B8A */
            r->a[1] = d0;                                      /* 0x196B90 */
            wr32(m, d0 + 0x48u, rd32(m, d0 + 0x48u) + 0x00007530u); /* 0x196B92 */
        }
        wr32(m, F3_A5 + 0x0886u, rd32(m, F3_A5 + 0x0886u) + 0x00012C5Fu); /* 0x196B9A */
        wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) | 0x20u)); /* 0x196BA2 */
        wr16(m, F3_A5 + 0x4B20u, rd16(m, obj + 0x34u));        /* 0x196BA8 */
        wr32(m, F3_A5 + 0x4B1Cu, rd32(m, obj + 0x36u));        /* 0x196BAE */
        wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) | 0x04u)); /* 0x196BB4 */
        r->d[0] = Obj_Spawn(m, obj, 0x00179D92u, 5u, 2u, 0u);  /* 0x196BCE */
        wr16(m, obj + 0x06u, 0x0002u);                         /* 0x196BD8 (beq never taken) */
        wr32(m, obj + 0x00u, 0x00196BE8u);                     /* 0x196BE0 */
        return 0;
    }

    if (sub_196AE0_regs(m, r, obj) < 0)                        /* 0x196C48 */
        return -1;

    wr32(m, obj + 0x00u, 0x00196C40u);                         /* 0x196C4C -> 0x196C38 */
    return 0;
}
