#include "obj_CDE50.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* cmp2.w (addr),Dn; bcs -> outside the signed bounds {lo.w, hi.w} at addr */
static int out_of_bounds(const f3mem_t *m, uint32_t addr, uint16_t v)
{
    int16_t lo = (int16_t)rd16(m, addr);
    int16_t hi = (int16_t)rd16(m, addr + 2u);
    return (int16_t)v < lo || (int16_t)v > hi;
}

/* ROM 0x0CE132: unpack packed trail history slot from parent +0x44. */
static uint32_t unpack_trail(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t idx = (uint16_t)((rd16(m, a1 + 0x3Au) - 2u) & 3u);
    uint32_t d0 = rd32(m, a1 + 0x4Cu + (uint32_t)idx * 4u);

    wr16(m, obj + 0x14u, (uint16_t)((int32_t)d0 >> 22));
    wr16(m, obj + 0x18u, (uint16_t)((int32_t)(d0 << 10) >> 23));
    wr16(m, obj + 0x20u, (uint16_t)((d0 >> 5) & 0xFFu));
    wr16(m, obj + 0x28u, (uint16_t)((d0 >> 4) & 1u));
    wr8(m, obj + 0x3Fu, (uint8_t)(rd8(m, obj + 0x3Fu) | ((d0 >> 2) & 3u)));
    return d0;
}

/* ROM 0x0CE16E: pack object state into trail ring buffer at +0x4C, preserving
 * the low 2 bits of d0 (from the caller's D0) in the bfins chain. */
static void pack_trail(const f3mem_t *m, uint32_t obj, uint32_t d0_low2)
{
    uint32_t d0 =
        (((uint32_t)rd16(m, obj + 0x14u) & 0x3FFu) << 22) |
        (((uint32_t)rd16(m, obj + 0x18u) & 0x1FFu) << 13) |
        (((uint32_t)rd16(m, obj + 0x20u) & 0x0FFu) << 5)  |
        (((uint32_t)rd16(m, obj + 0x28u) & 0x001u) << 4)  |
        (((uint32_t)rd8(m, obj + 0x3Fu)  & 0x003u) << 2)  |
        (d0_low2 & 3u);

    uint16_t idx = rd16(m, obj + 0x3Au);
    wr32(m, obj + 0x4Cu + (uint32_t)idx * 4u, d0);
    wr16(m, obj + 0x3Au, (uint16_t)((idx + 1u) & 3u));
}

/* ROM 0x0CDFCD..0CE0D6: walk the +0x44 chain, steering the head toward +0x0C
 * and replaying trail history for followers, then drawing each segment. */
static void update_chain(const f3mem_t *m, uint32_t obj)
{
    uint32_t cur = obj;
    do {
        uint32_t d0_pack;
        if (rd32(m, cur + 0x44u) != 0u) {
            d0_pack = unpack_trail(m, cur);        /* 0x0CDFD4 */
        } else {
            Obj_MoveByAngle(m, cur, rd16(m, cur + 0x20u)); /* 0x0CDFE0 */
            uint16_t ang = Obj_0CC3C6(m, cur, rd32(m, cur + 0x0Cu)); /* 0x0CDFEA */
            if (rd8(m, F3_A5 + 0x1D58u) & 0x02u)
                ang = (uint16_t)((ang & 0xFF00u) | (uint8_t)(ang - 0x80u));

            uint16_t diff_w = (uint16_t)(ang - rd16(m, cur + 0x20u));
            int8_t d0b = (int8_t)(diff_w & 0xFFu);
            int8_t d1b = d0b;
            if (d0b > 2)
                d0b = 2;
            else if (d0b < -2)
                d0b = -2;

            wr8(m, cur + 0x21u, (uint8_t)(rd8(m, cur + 0x21u) + (uint8_t)d0b));
            int16_t d1w = (int16_t)d1b;
            if (d1w < 0)
                d1w = (int16_t)(-d1w);
            if (d1w < 0x20) {
                wr8(m, cur + 0x3Eu, 0xFFu);
                if ((int16_t)rd16(m, cur + 0x42u) < 0x18)
                    wr16(m, cur + 0x42u, (uint16_t)(rd16(m, cur + 0x42u) + 2u));
            } else if (rd8(m, cur + 0x3Eu) == 0u) {
                int16_t spd = (int16_t)(rd16(m, cur + 0x42u) - 2u);
                if (spd <= 4)
                    spd = 4;
                wr16(m, cur + 0x42u, (uint16_t)spd);
            }
            d0_pack = (uint32_t)(uint8_t)d0b;
        }

        pack_trail(m, cur, d0_pack);               /* 0x0CE06C */

        uint16_t saved_x = rd16(m, cur + 0x14u);
        uint16_t saved_y = rd16(m, cur + 0x18u);
        uint16_t saved_spd = rd16(m, cur + 0x42u);

        wr16(m, cur + 0x14u, (uint16_t)(saved_x - 8u));
        wr16(m, cur + 0x18u, (uint16_t)(saved_y - 8u));
        uint16_t ang20 = rd16(m, cur + 0x20u);
        wr16(m, cur + 0x42u, 0u);
        Obj_MoveByAngle(m, cur, ang20);            /* 0x0CE08E */

        uint8_t d0b2 = (uint8_t)((ang20 & 0xFFu) + 3u);
        uint16_t d1_flip = (uint16_t)((d0b2 >> 6) & 3u);
        uint16_t d0_spr = (uint16_t)((d0b2 >> 3) & 0xFu);
        wr32(m, cur + 0x10u, rd32(m, 0x000CE0D8u + (uint32_t)d0_spr * 4u));
        wr8(m, cur + 0x1Cu, rd8(m, 0x000CC10Eu + d1_flip));
        Obj_DrawSprite(m, cur);                    /* 0x0CE0B6 */

        wr16(m, cur + 0x42u, saved_spd);
        wr16(m, cur + 0x18u, saved_y);
        wr16(m, cur + 0x14u, saved_x);

        cur = rd32(m, cur + 0x44u);
    } while (cur != 0u);
}

/* ROM 0x0CDE50: linked-segment missile/beam initialization and first tick. */
int Obj_CDE50_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);                       /* 0x0CDE50 */

    uint32_t d0 = rd32(m, obj + 0x0Cu);
    if (d0 != 0u)
        wr32(m, d0 + 0x48u, obj);                  /* 0x0CDE60 */
    wr32(m, obj + 0x44u, d0);                      /* 0x0CDE64 */
    wr32(m, obj + 0x48u, 0u);                      /* 0x0CDE68 */

    uint32_t a1 = rd32(m, obj + 0x08u);
    uint16_t seg_idx = rd16(m, a1 + 0x22u);
    wr16(m, a1 + 0x22u, (uint16_t)(seg_idx + 1u));
    wr16(m, obj + 0x22u, seg_idx);

    uint32_t d0_trail;
    if (seg_idx == 0u) {
        wr16(m, obj + 0x20u, rd16(m, a1 + 0x20u));
        wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x10u));
        wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x10u));
        d0_trail = 0u;
    } else {
        d0_trail = unpack_trail(m, obj);           /* 0x0CDEA2 */
    }

    for (int i = 0; i < 4; i++)
        pack_trail(m, obj, d0_trail);              /* 0x0CDEAA */

    wr32(m, obj + 0x28u, 0x00010000u);             /* 0x0CDEB2 */
    wr16(m, obj + 0x2Cu, (uint16_t)-4);            /* 0x0CDEBA */
    wr16(m, obj + 0x30u, (uint16_t)-4);            /* 0x0CDEC0 */
    wr16(m, obj + 0x2Eu, 3u);                      /* 0x0CDEC6 */
    wr16(m, obj + 0x32u, 3u);                      /* 0x0CDECC */
    wr16(m, obj + 0x42u, 0x0015u);                 /* 0x0CDED2 */
    wr8(m, obj + 0x3Eu, 0u);                       /* 0x0CDED8 */
    wr8(m, obj + 0x3Fu, 0u);                       /* 0x0CDEDC */
    wr8(m, obj + 0x1Du, (uint8_t)(seg_idx * 2u + 0xB0u)); /* 0x0CDEEA */
    wr32(m, obj + 0x0Cu, rd32(m, F3_A5 + 0x1D28u)); /* 0x0CDEEE */

    /* Falls through to 0x0CDEF4 */
    if (rd8(m, F3_A5 - 0x144Bu) & 0x10u) {
        wr8(m, obj + 0x3Fu, (uint8_t)(rd8(m, obj + 0x3Fu) | 0x03u));
        wr16(m, obj + 0x06u, 4u);
        wr32(m, obj + 0x00u, 0x000CDF84u);
        return 0;
    }

    if (rd32(m, obj + 0x48u) == 0u)
        update_chain(m, obj);                      /* 0x0CDF06 */

    if (rd32(m, obj + 0x44u) == 0u) {
        if (rd8(m, F3_A5 + 0x1D58u) & 0x04u) {
            wr8(m, obj + 0x3Fu, (uint8_t)(rd8(m, obj + 0x3Fu) | 0x03u));
            wr16(m, obj + 0x06u, 4u);
            wr32(m, obj + 0x00u, 0x000CDF84u);
            return 0;
        }
        if (out_of_bounds(m, 0x000CE1AAu, rd16(m, obj + 0x14u)) ||
            out_of_bounds(m, 0x000CE1AEu, rd16(m, obj + 0x18u))) {
            wr8(m, obj + 0x3Fu, (uint8_t)(rd8(m, obj + 0x3Fu) | 0x02u));
            wr16(m, obj + 0x06u, 4u);
            wr32(m, obj + 0x00u, 0x000CDF84u);
            return 0;
        }
    }

    if (rd8(m, obj + 0x3Fu) & 0x01u) {
        wr8(m, obj + 0x3Fu, (uint8_t)(rd8(m, obj + 0x3Fu) | 0x03u));
        wr16(m, obj + 0x06u, 4u);
        wr32(m, obj + 0x00u, 0x000CDF84u);
        return 0;
    }
    if (rd8(m, obj + 0x3Fu) & 0x02u) {
        wr8(m, obj + 0x3Fu, (uint8_t)(rd8(m, obj + 0x3Fu) | 0x02u));
        wr16(m, obj + 0x06u, 4u);
        wr32(m, obj + 0x00u, 0x000CDF84u);
        return 0;
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u)
        Coll_AddHazard(m, obj);                    /* 0x0CDF5A */

    wr32(m, obj + 0x00u, 0x000CDF68u);             /* 0x0CDF60 */
    return 0;
}
