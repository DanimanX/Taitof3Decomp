#include "obj_113ED0.h"
#include "../objects/obj_pool.h"
#include "../player/player_move.h"

/* ROM 0x006422: angle (0..255) of the vector (D0,D1) */
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
    d0 = (uint16_t)(d0 + rd16(m, 0x006492u + (uint32_t)(int32_t)(int16_t)d2));
    return (uint16_t)(d0 & 0x00FFu);
}

/* ROM 0x0CBE34: hit flash */
static void sub_0CBE34(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) == 0u) {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    } else if (rd8(m, obj + 0x1Du) == 0x40u) {
        wr8(m, obj + 0x35u, 0u);
        wr8(m, obj + 0x1Du, 0x42u);
    } else {
        wr8(m, obj + 0x1Du, 0x40u);
    }
}

/* ROM 0x0CBEB0: D0w scaled by the difficulty (A5+0x87E) */
static uint16_t sub_0CBEB0(const f3mem_t *m, uint16_t d0)
{
    uint16_t d1 = (uint16_t)((uint16_t)(rd16(m, F3_A5 + 0x087Eu) << 3) + 0x0C00u);
    uint32_t res = (uint32_t)((int32_t)(int16_t)d0 * (int32_t)(int16_t)d1) << 4;
    return (uint16_t)(res >> 16);
}

/* ROM 0x0CC2BC: D0w = (A5+0x878) mod max(1, ((0x180 - A5+0x87E) * d0) >> 8) */
static uint16_t sub_0CC2BC(const f3mem_t *m, uint16_t d0w)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = ((uint32_t)d1w * (uint32_t)d0w) << 8;
    d1w = (uint16_t)(d1 >> 16);
    if (d1w == 0u)
        d1w = 1u;
    return (uint16_t)(rd16(m, F3_A5 + 0x0878u) % d1w);
}

/* ROM 0x113456: random drift towards the screen centre */
static void sub_113456(const f3mem_t *m, uint32_t obj)
{
    if (!(rd8(m, F3_A5 + 0x1D58u) & 0x02u))
        return;
    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    if (x < -0x0A || x > 0x11C)
        return;
    /* 0x113478 */
    uint16_t save = rd16(m, obj + 0x42u);
    uint16_t r = (uint16_t)Rng_NextMem(m);
    wr16(m, obj + 0x42u, (uint16_t)((r & 0x000Fu) - 5u));
    uint16_t d0 = sub_006422(m, (uint16_t)(rd16(m, obj + 0x14u) - 0xA0u),
                             (uint16_t)(rd16(m, obj + 0x18u) - 0x74u));
    d0 = (uint16_t)((d0 & 0xFF00u) | (uint8_t)(d0 + 0x58u));
    Obj_MoveByAngle(m, obj, d0);
    wr16(m, obj + 0x42u, save);
}

/* ROM 0x113ED0 */
int Obj_113ED0_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) || (int32_t)rd32(m, obj + 0x24u) <= 0) {
        /* 0x114048 */
        wr8(m, obj + 0x51u, 0u);
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        wr8(m, obj + 0x50u, (uint8_t)(rd8(m, obj + 0x50u) | 0x10u));
        wr16(m, obj + 0x06u, 0x0078u);
        wr32(m, obj + 0x00u, 0x00114068u);
        return 0;
    }
    if (rd8(m, obj + 0x50u) & 0x08u) {
        /* 0x113FA6 */
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        wr16(m, obj + 0x06u, 0x04B0u);
        wr32(m, obj + 0x00u, 0x00113FBAu);
        return 0;
    }
    sub_0CBE34(m, obj);

    int clear51 = 1;
    if (!(rd8(m, obj + 0x50u) & 0x01u)) {
        uint16_t c = (uint16_t)(rd16(m, obj + 0x56u) - 1u);
        wr16(m, obj + 0x56u, c);
        uint16_t d0;
        if ((int16_t)c > 0) {
            /* 0x113F16 */
            d0 = sub_006422(m, (uint16_t)(rd16(m, obj + 0x52u) - rd16(m, obj + 0x14u)),
                            (uint16_t)(rd16(m, obj + 0x54u) - rd16(m, obj + 0x18u)));
        } else {
            if ((int16_t)rd16(m, obj + 0x14u) >= 0x1A4) {
                /* 0x11406E */
                wr8(m, obj + 0x50u, (uint8_t)(rd8(m, obj + 0x50u) | 0x20u));
                Obj_Free(m, rd32(m, obj + 0x08u));
                wr16(m, obj + 0x06u, 0x0002u);
                wr32(m, obj + 0x00u, 0x00114092u);
                return 0;
            }
            d0 = 0x00F0u;
        }
        /* 0x113F2C */
        d0 = (uint16_t)(d0 - rd16(m, obj + 0x20u));
        if ((int8_t)(uint8_t)d0 < 0)
            wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 2u));
        else
            wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) + 2u));
        /* 0x113F42 */
        Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));
        uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
        wr16(m, obj + 0x06u, t);
        if ((int16_t)t > 0 || (int16_t)rd16(m, obj + 0x56u) <= 0) {
            clear51 = 1;
        } else {
            clear51 = 0;
            if (sub_0CC2BC(m, 4u) == 0u) {
                uint8_t n = (uint8_t)((rd8(m, obj + 0x51u) + 1u) & 0x07u);
                wr8(m, obj + 0x51u, n);
                if (n == 0u)
                    wr16(m, obj + 0x06u, sub_0CBEB0(m, 0x0028u));
            }
        }
    }
    if (clear51)
        wr8(m, obj + 0x51u, 0u);            /* 0x113F8C */

    /* 0x113F90 */
    sub_113456(m, obj);
    if ((rd16(m, F3_A5 + 0x0878u) & 0x7Fu) == 0u) {
        /* 0x113EB0 */
        uint16_t r = (uint16_t)Rng_NextMem(m);
        wr16(m, obj + 0x52u, (uint16_t)((r & 0x7Fu) + 0xA0u));
        wr16(m, obj + 0x54u, 0x0046u);
    }
    wr32(m, obj + 0x00u, 0x00113ED0u);
    return 0;
}
