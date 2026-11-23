#include "obj_108DFC.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"

/* ROM 0x0CEC4C */
static void sub_0CEC4C(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    a1 += 0x14u;
    wr32(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x24u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x28u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1)); a1 += 4u;
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);

    uint16_t d0 = rd8(m, obj + 0x05u);
    if (rd8(m, obj + 0x04u) != 0x05u)
        d0 = (uint16_t)(d0 + 0x20u);
    uint32_t a2 = rd32(m, 0x000CECE4u + (uint32_t)d0 * 4u);
    wr32(m, obj + 0x0Cu, a2);

    wr32(m, a2 + 0x08u, 0u);
    wr16(m, a2 + 0x08u, rd16(m, a1)); a1 += 2u;
    wr32(m, a2 + 0x0Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, a2 + 0x10u, 0u);
    wr16(m, a2 + 0x10u, rd16(m, a1)); a1 += 2u;
    wr32(m, a2 + 0x14u, rd32(m, a1)); a1 += 4u;
    wr32(m, a2 + 0x18u, 0u);
    wr16(m, a2 + 0x18u, rd16(m, a1)); a1 += 2u;
    wr32(m, a2 + 0x1Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, a2 + 0x20u, 0u);
    wr16(m, a2 + 0x20u, rd16(m, a1)); a1 += 2u;
    wr32(m, a2 + 0x24u, rd32(m, a1));
}

/* ROM 0x0CEE2C: palette index from the byte table 0x0CEE40 */
static void sub_0CEE2C(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = rd8(m, obj + 0x4Eu);
    uint8_t b = rd8(m, 0x000CEE40u + d0);
    wr8(m, obj + 0x1Du, b);
    wr8(m, obj + 0x4Fu, b);
}

/* ROM 0x0CF1D8: script pointer + "run script" bit */

/* ROM 0x0D0E3A: 4-way interleaved global counter in A5+0x1D60 */

/* ROM 0x0D0E52 */
static uint16_t sub_0D0E52(const f3mem_t *m, int16_t d0_in, int16_t d1_in)
{
    uint16_t d0 = (uint16_t)d0_in;
    uint16_t d1 = (uint16_t)d1_in;
    uint16_t d2 = 0u;

    if ((int16_t)d0 < 0) {
        d2 = (uint16_t)(d2 + 2u);
        d0 = (uint16_t)(0u - d0);
    }
    if ((int16_t)d1 < 0) {
        d2 = (uint16_t)(d2 + 4u);
        d1 = (uint16_t)(0u - d1);
    }

    uint16_t v = (d1 <= d0) ? d0 : d1;
    uint16_t d3;
    if (v == 0u) {
        d3 = 32u;
    } else {
        d3 = 16u;
        while (!(v & 0x8000u)) {
            v = (uint16_t)(v << 1);
            d3++;
        }
    }

    d3 = (uint16_t)(d3 - 0x1Au);
    if ((int16_t)d3 < 0) {
        d3 = (uint16_t)(0u - d3);
        d0 = (uint16_t)(d0 >> d3);
        d1 = (uint16_t)(d1 >> d3);
    }

    if (d2 != 2u && d2 != 4u) {
        uint16_t tmp = d0;
        d0 = d1;
        d1 = tmp;
    }

    d1 = (uint16_t)((uint16_t)(d1 << 6) + d0);
    d0 = rd8(m, 0x000D0EBCu + (uint32_t)(int32_t)(int16_t)d1);
    d0 = (uint16_t)(d0 + rd16(m, 0x000D0EB4u + (uint32_t)(int32_t)(int16_t)d2));
    if (d0 == 0x168u)
        d0 = 0u;
    return d0;
}

/* part of ROM 0x0D06E0: angle/distance from the hit point (+0x68) to a player */
static uint16_t aim_player(const f3mem_t *m, uint32_t obj, uint32_t pl,
                           uint16_t *dx, uint16_t *dy)
{
    if ((int16_t)rd16(m, pl + 0x4Cu) < 0)
        return 0xFFFFu;
    uint32_t a2 = rd32(m, obj + 0x68u);
    uint16_t x = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, pl + 0x14u) + rd16(m, a2) - 0x17u);
    uint16_t y = (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, pl + 0x18u) + rd16(m, a2 + 2u) - 0x0Fu);
    *dx = ((int16_t)x < 0) ? (uint16_t)(0u - x) : x;
    *dy = ((int16_t)y < 0) ? (uint16_t)(0u - y) : y;
    return sub_0D0E52(m, (int16_t)x, (int16_t)y);
}

/* ROM 0x0D06E0: +0x42 = angle to the nearest live player (0xB4 when none) */
static void sub_0D06E0(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x42u, 0x00B4u);
    uint16_t d2 = 0, d3 = 0, d5 = 0, d6 = 0;
    uint16_t d4 = aim_player(m, obj, 0x41B190u, &d2, &d3);
    uint16_t d7 = aim_player(m, obj, 0x41B21Au, &d5, &d6);
    if ((int16_t)d4 < 0) {
        if ((int16_t)d7 >= 0)
            wr16(m, obj + 0x42u, d7);
        return;
    }
    if ((int16_t)d7 < 0) {
        wr16(m, obj + 0x42u, d4);
        return;
    }
    if (d3 > d2) d2 = d3;
    if (d6 > d5) d5 = d6;
    wr16(m, obj + 0x42u, (d5 < d2) ? d7 : d4);
}

/* ROM 0x108DFC */
int Obj_108DFC_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0CEC4C(m, obj, 0x001092CEu);        /* lea (0x4d0,PC) */
    sub_0CEE2C(m, obj);
    wr32(m, obj + 0x58u, 0x00B40000u);
    Obj_StartScript(m, obj, 0x0010933Eu);        /* lea (0x528,PC) */
    wr32(m, obj + 0x3Au, 0x01400000u);
    wr32(m, obj + 0x3Eu, 0xFFE00000u);
    wr32(m, obj + 0x14u, 0x01400000u);
    wr32(m, obj + 0x18u, 0x00740000u);
    wr32(m, obj + 0x68u, 0x0010947Au);
    sub_0D06E0(m, obj);
    if (rd16(m, obj + 0x42u) < 0x00B4u) {
        wr32(m, obj + 0x3Eu, 0x00E80000u);
        Obj_StartScript(m, obj, 0x0010938Cu);    /* lea (0x52c,PC) */
    }

    /* 0x108E68 */
    uint16_t d0 = (uint16_t)((Obj_Next4Way(m) & 0x3Fu) - 0x20u);
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + d0));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));
    wr32(m, obj + 0x00u, 0x00108E8Eu);
    return 0;
}
