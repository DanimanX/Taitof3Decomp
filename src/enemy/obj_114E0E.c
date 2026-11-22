#include "obj_114E0E.h"
#include "../objects/obj_pool.h"
#include "../player/player_move.h"

/* ROM 0x006422: angle (0..255) of the vector (D0.w, D1.w) */
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
        unsigned cnt = (unsigned)d3 & 63u;
        d0 = (cnt >= 16u) ? 0u : (uint16_t)(d0 >> cnt);
        d1 = (cnt >= 16u) ? 0u : (uint16_t)(d1 >> cnt);
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

/* ROM 0x0CBE34: damage flash colour in +0x1D */
static void sub_0CBE34(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) != 0u) {
        if (rd8(m, obj + 0x1Du) == 0x40u) {
            wr8(m, obj + 0x35u, 0u);
            wr8(m, obj + 0x1Du, 0x42u);
        } else {
            wr8(m, obj + 0x1Du, 0x40u);
        }
    } else {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    }
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
    uint16_t d0 = sub_006422(m, (uint16_t)(rd16(m, obj + 0x14u) - 0x00A0u),
                             (uint16_t)(rd16(m, obj + 0x18u) - 0x0074u));
    d0 = (uint16_t)((d0 & 0xFF00u) | (uint8_t)((uint8_t)d0 + 0x58u));
    Obj_MoveByAngle(m, obj, d0);
    wr16(m, obj + 0x42u, save);
}

/* ROM 0x114E0E */
int Obj_114E0E_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) || (int32_t)rd32(m, obj + 0x24u) <= 0) {
        /* 0x114F08: hp depleted (or off) -> leave */
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        wr8(m, obj + 0x50u, (uint8_t)(rd8(m, obj + 0x50u) | 0x10u));
        wr16(m, obj + 0x06u, 0x0078u);
        if (rd8(m, obj + 0x50u) & 0x10u) {          /* result of bset.b #4 */
            wr32(m, obj + 0x00u, 0x00114F24u);
            return 0;
        }
        /* 0x114F2A */
        wr8(m, obj + 0x50u, (uint8_t)(rd8(m, obj + 0x50u) | 0x20u));
        wr32(m, obj + 0x00u, 0x00114F38u);
        return 0;
    }

    if (rd8(m, obj + 0x50u) & 0x08u) {
        /* 0x114E90 */
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        wr16(m, obj + 0x42u, 0x0006u);
        wr16(m, obj + 0x06u, 0x04B0u);
        wr32(m, obj + 0x00u, 0x00114EAAu);
        return 0;
    }

    sub_0CBE34(m, obj);
    if (rd8(m, obj + 0x50u) & 0x01u) {
        wr32(m, obj + 0x00u, 0x00114E0Eu);          /* 0x114E06 */
        return 0;
    }

    int16_t speed = (int16_t)rd16(m, obj + 0x42u);
    if (speed >= 0x0A) {
        /* 0x114E4A: leave the screen */
        if ((int16_t)rd16(m, obj + 0x14u) <= -0x64)
            goto leave;
        if ((int16_t)rd16(m, obj + 0x14u) >= 0x1A4)
            goto leave;
        if ((int16_t)rd16(m, obj + 0x18u) <= -0x32)
            goto leave;
        if ((int16_t)rd16(m, obj + 0x18u) >= 0x11A)
            goto leave;
    } else {
        wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) + 1u));
    }

    /* 0x114E72 */
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x22u) >> 2));
    Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));
    sub_113456(m, obj);
    wr32(m, obj + 0x00u, 0x00114E0Eu);              /* 0x114E06 */
    return 0;

leave:
    /* 0x114F2A */
    wr8(m, obj + 0x50u, (uint8_t)(rd8(m, obj + 0x50u) | 0x20u));
    wr32(m, obj + 0x00u, 0x00114F38u);
    return 0;
}
