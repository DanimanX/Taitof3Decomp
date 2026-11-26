/* ROM 0x19A1E6: boss sub-part sprite drawer and 3-shot burst launcher (A0 = obj).
 * If A5+0x4B38 == -1, sets +0x06=2 and transitions to 0x17A85C. Otherwise
 * updates world/screen position from parent (+0x08) + (+0x16,+0x1A), adjusts
 * screen (+0x14,+0x18) by sprite half-size*8, draws sprite (0x001282), and if
 * A5+0x4BA8 bit 6 from MSB is clear re-arms 0x19A1E6; else spawns 3 projectiles
 * (0x182D24, group 3, count 2) from table 0x19A3E2 or 0x19A3EE (if player
 * A5+0x1D28 X+0x18 >= +0x50), sets +0x64=0xA0000 and advances to 0x19A290. */
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

int Fn_19A1E6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (rd16(m, F3_A5 + 0x4B38u) == 0xFFFFu) {           /* 0x19A1E6..0x19A1EC */
        wr16(m, obj + 0x06u, 0x0002u);                   /* 0x17A84C (0x17A852 beq never taken) */
        wr32(m, obj, 0x0017A85Cu);                       /* 0x17A854 */
        return 0;
    }

    uint32_t a4 = rd32(m, obj + 0x08u);                  /* 0x19A1F2 */
    r->a[4] = a4;
    uint16_t x = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t y = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, x);                             /* 0x19A206 */
    wr16(m, obj + 0x54u, y);                             /* 0x19A20A */
    wr16(m, obj + 0x14u, x);                             /* 0x19A20E */
    wr16(m, obj + 0x18u, y);                             /* 0x19A212 */

    uint32_t a1 = rd32(m, obj + 0x10u);                  /* 0x19A216 */
    uint16_t sy = (uint16_t)(rd16(m, a1) << 3);          /* 0x19A21A, 0x19A220 */
    uint16_t sx = (uint16_t)(rd16(m, a1 + 2u) << 3);     /* 0x19A21C, 0x19A21E */
    wr16(m, obj + 0x14u, (uint16_t)(x - sx));            /* 0x19A222 */
    wr16(m, obj + 0x18u, (uint16_t)(y - sy));            /* 0x19A226 */

    draw_sprite_regs(m, r);                              /* 0x19A22A */

    if ((rd8(m, F3_A5 + 0x4BA8u) & 0x02u) == 0u) {       /* 0x19A230 bftst {6:1} */
        wr32(m, obj, 0x0019A1E6u);                       /* 0x19A1DE */
        return 0;
    }

    uint32_t a2 = 0x0019A3E2u;                           /* 0x19A23A */
    int32_t p = (int32_t)rd32(m, F3_A5 + 0x1D28u);       /* 0x19A240 */
    r->d[0] = (uint32_t)p;
    if (p > 0) {                                         /* 0x19A244 */
        r->a[1] = (uint32_t)p;                           /* 0x19A248 */
        int16_t px = (int16_t)(rd16(m, (uint32_t)p + 0x14u) + 0x0018); /* 0x19A24A..0x19A24E */
        r->d[0] = setw(r->d[0], (uint16_t)px);
        if (px >= (int16_t)rd16(m, obj + 0x50u))         /* 0x19A252..0x19A256 */
            a2 = 0x0019A3EEu;                            /* 0x19A25A */
    }

    for (int i = 0; i < 3; i++) {                        /* 0x19A238, 0x19A260..0x19A27C */
        uint32_t arg = rd32(m, a2);
        a2 += 4u;
        r->d[1] = arg;
        r->d[0] = Obj_Spawn(m, obj, 0x00182D24u, 3u, 2u, arg);
    }
    r->d[2] = setw(r->d[2], 0xFFFFu);
    r->a[2] = a2;

    wr32(m, obj + 0x64u, 0x000A0000u);                   /* 0x19A280 */
    wr32(m, obj, 0x0019A290u);                           /* 0x19A288 */
    return 0;
}
