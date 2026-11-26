/* ROM 0x103714: 10-turret boss pod row tick (A0 = object, A0+0x08 = parent).
 * Copies (+0x3A, +0x3E), +0x1D, +0x22 from parent; if +0x22 == 0, transfers to
 * Obj_StartDying (0x0CEE74). Advances 10 pairs of byte animation counters at
 * (+0x0C), decrements +0x23 cooldown if nonzero, draws 3 ancestor sprites
 * (+0x08 chain depth 4, 3, 2), then for each of 10 turrets sets sprite frame
 * and draws upper/lower parts around (+0x3A, +0x3E) + (+0x68). When +0x23 == 0
 * on d7 < 5, sets +0x68 = 0x102F4E, aims at players via 0x0D09B8 (0x1038EE), and
 * if +0x23 fired spawns 0x103BAA shot. Ends by drawing parent+0x08 sprite. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
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

int Fn_103714(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t parent = rd32(m, obj + 0x08u);

    wr32(m, obj + 0x3Au, rd32(m, parent + 0x3Au));        /* 0x103718 */
    wr32(m, obj + 0x3Eu, rd32(m, parent + 0x3Eu));        /* 0x10371E */
    wr8(m, obj + 0x1Du, rd8(m, parent + 0x1Du));          /* 0x103724 */
    uint8_t alive = rd8(m, parent + 0x22u);
    wr8(m, obj + 0x22u, alive);                           /* 0x10372A */
    if (alive == 0u) {
        r->a[1] = parent;
        Obj_StartDying(m, obj);                           /* 0x103730 -> 0x0CEE74 */
        return 0;
    }

    uint32_t a1 = rd32(m, obj + 0x0Cu);                   /* 0x103738 */
    for (int16_t d7 = 9; d7 >= 0; d7--) {
        uint8_t c0 = (uint8_t)(rd8(m, a1) - 1u);
        wr8(m, a1, c0);
        if (c0 == 0u) {
            wr8(m, a1, 0x0Fu);
            uint8_t f0 = (uint8_t)(rd8(m, a1 + 1u) + 1u);
            wr8(m, a1 + 1u, f0);
            if (f0 > 0x0Bu)
                wr8(m, a1 + 1u, 0u);
        }
        a1 += 2u;

        uint8_t c1 = (uint8_t)(rd8(m, a1) - 1u);
        wr8(m, a1, c1);
        if (c1 == 0u) {
            wr8(m, a1, 0x0Au);
            uint8_t f1 = (uint8_t)(rd8(m, a1 + 1u) + 1u);
            wr8(m, a1 + 1u, f1);
            if (f1 > 0x0Du)
                wr8(m, a1 + 1u, 0u);
        }
        a1 += 2u;
    }

    if (rd8(m, obj + 0x23u) != 0u)                        /* 0x10377C */
        wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) - 1u));

    /* 0x103788..0x1037B4: draw ancestors at depth 4, 3, 2 */
    uint32_t anc1 = rd32(m, obj + 0x08u);
    uint32_t anc2 = rd32(m, anc1 + 0x08u);
    uint32_t anc3 = rd32(m, anc2 + 0x08u);
    uint32_t anc4 = rd32(m, anc3 + 0x08u);
    Obj_DrawSprite(m, anc4);
    Obj_DrawSprite(m, anc3);
    Obj_DrawSprite(m, anc2);

    uint32_t a4 = rd32(m, obj + 0x0Cu);                   /* 0x1037B8 */
    for (int16_t d7 = 9; d7 >= 0; d7--) {
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));       /* 0x1037BC */
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));       /* 0x1037C2 */

        uint16_t f0 = rd16(m, a4) & 0x00FFu;              /* 0x1037C8 */
        a4 += 2u;
        wr16(m, obj + 0x44u, f0);                         /* 0x1037CE */
        uint32_t spr0 = rd32(m, ((d7 >= 5) ? 0x001039D6u : 0x00103A3Eu)
                                + (uint32_t)f0 * 4u);
        Obj_SetFrame(m, obj, spr0);                       /* 0x1037EA */

        uint32_t off0 = 0x0010394Eu + (uint32_t)d7 * 4u;
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u)
                                        + rd16(m, off0) + rd16(m, obj + 0x68u)));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u)
                                        + rd16(m, off0 + 2u)));
        Obj_DrawSprite(m, obj);                           /* 0x10380A */

        uint16_t f1 = rd16(m, a4) & 0x00FFu;              /* 0x103812 */
        a4 += 2u;
        wr16(m, obj + 0x46u, f1);                         /* 0x103818 */
        uint32_t spr1 = rd32(m, ((d7 >= 5) ? 0x00103A06u : 0x00103A6Eu)
                                + (uint32_t)f1 * 4u);
        Obj_SetFrame(m, obj, spr1);                       /* 0x103834 */

        uint16_t idx = rd16(m, obj + 0x44u);              /* 0x10383A */
        uint32_t off1 = ((d7 >= 5) ? 0x00103976u : 0x001039A6u)
                        + (uint32_t)idx * 4u;
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, off1)));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u)
                                        + rd16(m, off1 + 2u) + rd16(m, obj + 0x68u)));
        Obj_DrawSprite(m, obj);                           /* 0x103868 */

        if (rd8(m, obj + 0x23u) == 0u && d7 < 5) {        /* 0x103870 */
            wr32(m, obj + 0x68u, 0x00102F4Eu);            /* 0x103884 */
            Obj_AimAtPlayers(m, obj, 0x001038EEu);        /* 0x103890 */
            if (rd8(m, obj + 0x23u) != 0u) {              /* 0x10389A */
                int32_t child = (int32_t)Obj_Spawn(m, obj, 0x00103BAAu, 9u, 2u, 0u);
                if (child >= 0) {
                    uint32_t c = (uint32_t)child;
                    wr32(m, c + 0x14u, rd32(m, obj + 0x14u));
                    wr32(m, c + 0x18u, rd32(m, obj + 0x18u));
                    wr16(m, c + 0x10u,
                         rd16(m, 0x00102D5Eu + (uint32_t)rd16(m, obj + 0x46u) * 2u));
                }
            }
        }
    }

    r->a[0] = rd32(m, obj + 0x08u);                       /* 0x1038E2 */
    draw_sprite_regs(m, r);                               /* 0x1038E6 */
    return 0;
}
