#include "obj_CDF68.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"

/* ROM 0x006422 */
static uint16_t sub_006422(const f3mem_t *m, uint16_t d0, uint16_t d1)
{
    uint32_t a0 = 0x026E40u;
    uint16_t d2 = 0u;

    if ((int16_t)d0 < 0) {
        d2 |= 0x0002u;
        d0 = (uint16_t)(0u - d0);
    }
    if ((int16_t)d1 < 0) {
        d2 |= 0x0004u;
        d1 = (uint16_t)(0u - d1);
    }

    uint16_t src = ((int16_t)d1 > (int16_t)d0) ? d1 : d0;
    uint16_t d3 = 0x20u;
    for (uint16_t i = 0u; i < 16u; i++) {
        if (src & (uint16_t)(0x8000u >> i)) {
            d3 = (uint16_t)(0x10u + i);
            break;
        }
    }

    d3 = (uint16_t)(d3 - 0x1Au);
    if ((int16_t)d3 < 0) {
        d3 = (uint16_t)(0u - d3);
        d0 = (uint16_t)(d0 >> (d3 & 63u));
        d1 = (uint16_t)(d1 >> (d3 & 63u));
    }

    if (d2 != 0u && d2 != 6u) {
        uint16_t tmp = d0;
        d0 = d1;
        d1 = tmp;
    }

    d1 = (uint16_t)(d1 << 6);
    a0 += (uint32_t)(int32_t)(int16_t)d1;
    uint8_t tbl = rd8(m, a0 + (uint32_t)(int32_t)(int16_t)d0);
    d0 = (uint16_t)((d0 & 0xFF00u) | (uint16_t)tbl);
    a0 = 0x006492u;
    d0 = (uint16_t)(d0 + rd16(m, a0 + (uint32_t)(int32_t)(int16_t)d2));
    d0 &= 0x00FFu;
    return d0;
}
/* ROM 0x0CBDAE: append to the player-hit list (A5+0x906) */
static void sub_0CBDAE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x0906u, a1 + 4u);
}


static int out_of_bounds(const f3mem_t *m, uint32_t addr, uint16_t v)
{
    int16_t lo = (int16_t)rd16(m, addr), hi = (int16_t)rd16(m, addr + 2u);
    return (int16_t)v < lo || (int16_t)v > hi;
}

/* ROM 0x0CE16E: pack the position history entry */
static void sub_0CE16E(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    d0 = (d0 & 0x3u)
       | ((uint32_t)(rd16(m, obj + 0x14u) & 0x3FFu) << 22)
       | ((uint32_t)(rd16(m, obj + 0x18u) & 0x1FFu) << 13)
       | ((uint32_t)(rd16(m, obj + 0x20u) & 0xFFu) << 5)
       | ((uint32_t)(rd16(m, obj + 0x28u) & 0x1u) << 4)
       | ((uint32_t)(rd8(m, obj + 0x3Fu) & 0x3u) << 2);
    int16_t i = (int16_t)rd16(m, obj + 0x3Au);
    wr32(m, obj + 0x4Cu + (uint32_t)(int32_t)i * 4u, d0);
    wr16(m, obj + 0x3Au, (uint16_t)((rd16(m, obj + 0x3Au) + 1u) & 3u));
}

/* ROM 0x0CE132: follow the leader's history two entries back */
static uint32_t sub_0CE132(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t i = (uint16_t)((rd16(m, a1 + 0x3Au) - 2u) & 3u);
    uint32_t d0 = rd32(m, a1 + 0x4Cu + (uint32_t)i * 4u);
    wr16(m, obj + 0x14u, (uint16_t)(int16_t)((int32_t)d0 >> 22));
    wr16(m, obj + 0x18u, (uint16_t)(int16_t)((int32_t)(d0 << 10) >> 23));
    wr16(m, obj + 0x20u, (uint16_t)((d0 >> 5) & 0xFFu));
    wr16(m, obj + 0x28u, (uint16_t)((d0 >> 4) & 0x1u));
    wr8(m, obj + 0x3Fu, (uint8_t)(rd8(m, obj + 0x3Fu) | ((d0 >> 2) & 0x3u)));
    return d0;
}

/* ROM 0x0CC3C6: angle from A0 to A1 (box centres) */
static uint16_t sub_0CC3C6(const f3mem_t *m, uint32_t a0, uint32_t a1)
{
    uint16_t d0 = (uint16_t)(rd16(m, a1 + 0x2Cu) + rd16(m, a1 + 0x2Eu) - rd16(m, a0 + 0x2Cu) - rd16(m, a0 + 0x2Eu));
    d0 = (uint16_t)((int16_t)d0 >> 1);
    d0 = (uint16_t)(d0 + rd16(m, a1 + 0x14u) - rd16(m, a0 + 0x14u));
    uint16_t d1 = (uint16_t)(rd16(m, a1 + 0x30u) + rd16(m, a1 + 0x32u) - rd16(m, a0 + 0x30u) - rd16(m, a0 + 0x32u));
    d1 = (uint16_t)((int16_t)d1 >> 1);
    d1 = (uint16_t)(d1 + rd16(m, a1 + 0x18u) - rd16(m, a0 + 0x18u));
    return sub_006422(m, d0, d1);
}

/* ROM 0x0CDFC8: move/draw the chain from this node up to its head */
static void sub_0CDFC8(const f3mem_t *m, uint32_t a0)
{
    do {
        uint32_t d0;
        if (rd32(m, a0 + 0x44u) != 0u) {
            d0 = sub_0CE132(m, a0);
        } else {
            Obj_MoveByAngle(m, a0, rd16(m, a0 + 0x20u));
            uint16_t w = sub_0CC3C6(m, a0, rd32(m, a0 + 0x0Cu));
            if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u)
                w = (uint16_t)((w & 0xFF00u) | (uint8_t)(w + 0x80u));
            w = (uint16_t)(w - rd16(m, a0 + 0x20u));
            int8_t d1b = (int8_t)w;
            int8_t b = (int8_t)w;
            if (b > 2) b = 2;
            else if (b < -2) b = -2;
            d0 = (uint8_t)b;
            wr8(m, a0 + 0x21u, (uint8_t)(rd8(m, a0 + 0x21u) + (uint8_t)b));
            int16_t ad = d1b < 0 ? (int16_t)-d1b : d1b;
            if (ad < 0x20) {
                wr8(m, a0 + 0x3Eu, 0xFFu);
                if ((int16_t)rd16(m, a0 + 0x42u) < 0x18)
                    wr16(m, a0 + 0x42u, (uint16_t)(rd16(m, a0 + 0x42u) + 2u));
            } else if (rd8(m, a0 + 0x3Eu) == 0u) {
                uint16_t s = (uint16_t)(rd16(m, a0 + 0x42u) - 2u);
                wr16(m, a0 + 0x42u, s);
                if ((int16_t)s <= 4)
                    wr16(m, a0 + 0x42u, 4u);
            }
        }
        sub_0CE16E(m, a0, d0);
        uint16_t x = rd16(m, a0 + 0x14u), y = rd16(m, a0 + 0x18u), sp = rd16(m, a0 + 0x42u);
        wr16(m, a0 + 0x14u, (uint16_t)(x - 8u));
        wr16(m, a0 + 0x18u, (uint16_t)(y - 8u));
        uint16_t ang = rd16(m, a0 + 0x20u);
        wr16(m, a0 + 0x42u, 0u);
        Obj_MoveByAngle(m, a0, ang);
        uint8_t b = (uint8_t)(ang + 3u);
        wr32(m, a0 + 0x10u, rd32(m, 0xCE0D8u + (uint32_t)((b >> 3) & 0xFu) * 4u));
        wr8(m, a0 + 0x1Cu, rd8(m, 0xCC10Eu + ((b >> 6) & 3u)));
        Obj_DrawSprite(m, a0);
        wr16(m, a0 + 0x42u, sp);
        wr16(m, a0 + 0x18u, y);
        wr16(m, a0 + 0x14u, x);
        a0 = rd32(m, a0 + 0x44u);
    } while (a0 != 0u);
}

/* ROM 0x0CDF68 -> 0x0CDEF4: snake segment */
int Obj_CDF68_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)
        goto kill;
    if (rd32(m, obj + 0x48u) == 0u)
        sub_0CDFC8(m, obj);
    if (rd32(m, obj + 0x44u) == 0u) {
        if ((rd8(m, F3_A5 + 0x1D58u) & 0x04u) != 0u)
            goto kill;
        if (out_of_bounds(m, 0xCE1AAu, rd16(m, obj + 0x14u)) ||
            out_of_bounds(m, 0xCE1AEu, rd16(m, obj + 0x18u)))
            goto off;
    }
    if ((rd8(m, obj + 0x3Fu) & 0x01u) != 0u)
        goto kill;
    if ((rd8(m, obj + 0x3Fu) & 0x02u) != 0u)
        goto off;
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u)
        sub_0CBDAE(m, obj);
    wr32(m, obj, 0xCDF68u);
    return 0;
kill:
    wr8(m, obj + 0x3Fu, (uint8_t)(rd8(m, obj + 0x3Fu) | 0x01u));
off:
    wr8(m, obj + 0x3Fu, (uint8_t)(rd8(m, obj + 0x3Fu) | 0x02u));
    wr16(m, obj + 0x06u, 4u);
    wr32(m, obj, 0xCDF84u);
    return 0;
}
