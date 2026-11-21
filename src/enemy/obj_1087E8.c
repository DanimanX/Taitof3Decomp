#include "obj_1087E8.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

/* ROM 0x0D0C9E */
static uint16_t sub_0D0C9E(const f3mem_t *m)
{
    uint16_t d0 = (uint16_t)(rd8(m, F3_A5 + 0x87Fu) >> 3);
    return rd16(m, 0x000D0D42u + (uint32_t)d0 * 2u);
}

/* ROM 0x0D0E3A */
static uint16_t sub_0D0E3A(const f3mem_t *m)
{
    uint16_t d0 = rd16(m, F3_A5 + 0x1D60u);
    uint16_t d1 = (uint16_t)(d0 << 2);
    d0 = (uint16_t)(d0 + d1 + 1u);
    wr16(m, F3_A5 + 0x1D60u, d0);
    return (uint16_t)(d0 >> 8);
}

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

/* ROM 0x0D09B8: returns A2 (selected player struct) when a shot is triggered */
static uint32_t sub_0D09B8(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint32_t hit = rd32(m, obj + 0x68u);
    int16_t sx = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, hit));
    if (sx < 0x28 || sx > 0x127)
        return hit;
    int16_t sy = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, hit + 2u));
    if (sy < 0x10 || sy > 0xDF)
        return hit;

    uint16_t d2 = 0u, d3 = 0u, d4 = 0xFFFFu;
    if ((int16_t)rd16(m, 0x41B190u + 0x4Cu) >= 0) {
        uint32_t a4 = rd32(m, obj + 0x68u) + 2u;
        d3 = (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, 0x41B190u + 0x18u) + rd16(m, a4) - 0x0Fu);
        uint16_t a3w = d3;
        if ((int16_t)d3 < 0)
            d3 = (uint16_t)(0u - d3);
        if (d3 > rd16(m, a1 + 0x0Au))
            goto check_p2;

        d2 = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, 0x41B190u + 0x14u) + rd16(m, a4 - 2u) - 0x17u);
        uint16_t a4w = d2;
        if ((int16_t)d2 < 0) {
            d2 = (uint16_t)(0u - d2);
            if (d2 > rd16(m, a1 + 0x0Au))
                goto check_p2;
            if (d2 < rd16(m, a1 + 0x08u) && d3 < rd16(m, a1 + 0x08u))
                goto check_p2;
        } else {
            if (d2 > rd16(m, a1 + 0x0Au))
                goto check_p2;
            int16_t thresh = (int16_t)(rd16(m, a1 + 0x08u) - 0x1Eu);
            if ((int16_t)d2 < thresh && d3 < rd16(m, a1 + 0x08u))
                goto check_p2;
        }

        uint16_t ang = sub_0D0E52(m, (int16_t)a4w, (int16_t)a3w);
        uint16_t amin = rd16(m, a1 + 0x0Cu);
        uint16_t amax = rd16(m, a1 + 0x0Eu);
        if (amin > amax) {
            if (ang >= amin || ang <= amax)
                d4 = ang;
        } else {
            if (ang >= amin && ang <= amax)
                d4 = ang;
        }
    }

check_p2:;
    uint32_t a2 = 0x41B21Au;
    uint16_t d5 = 0u, d6 = 0u, d7 = 0xFFFFu;
    if ((int16_t)rd16(m, 0x41B21Au + 0x4Cu) >= 0) {
        uint32_t a4 = rd32(m, obj + 0x68u) + 2u;
        d6 = (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, 0x41B21Au + 0x18u) + rd16(m, a4) - 0x0Fu);
        uint16_t a3w = d6;
        if ((int16_t)d6 < 0)
            d6 = (uint16_t)(0u - d6);
        if (d6 > rd16(m, a1 + 0x0Au))
            goto check_best;

        d5 = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, 0x41B21Au + 0x14u) + rd16(m, a4 - 2u) - 0x17u);
        uint16_t a4w = d5;
        if ((int16_t)d5 < 0) {
            d5 = (uint16_t)(0u - d5);
            if (d5 > rd16(m, a1 + 0x0Au))
                goto check_best;
            if (d5 < rd16(m, a1 + 0x08u) && d6 < rd16(m, a1 + 0x08u))
                goto check_best;
        } else {
            if (d5 > rd16(m, a1 + 0x0Au))
                goto check_best;
            int16_t thresh = (int16_t)(rd16(m, a1 + 0x08u) - 0x1Eu);
            if ((int16_t)d5 < thresh && d6 < rd16(m, a1 + 0x08u))
                goto check_best;
        }

        uint16_t ang = sub_0D0E52(m, (int16_t)a4w, (int16_t)a3w);
        uint16_t amin = rd16(m, a1 + 0x0Cu);
        uint16_t amax = rd16(m, a1 + 0x0Eu);
        if (amin > amax) {
            if (ang >= amin || ang <= amax)
                d7 = ang;
        } else {
            if (ang >= amin && ang <= amax)
                d7 = ang;
        }
    }

check_best:;
    uint16_t d0 = d7;
    if ((int16_t)d4 < 0) {
        if ((int16_t)d7 < 0)
            return a2;
    } else {
        if ((int16_t)d7 < 0) {
            a2 = 0x41B190u;
            d0 = d4;
        } else {
            if (d3 > d2)
                d2 = d3;
            if (d6 > d5)
                d5 = d6;
            if (d5 >= d2) {
                a2 = 0x41B190u;
                d0 = d4;
            }
        }
    }

    wr16(m, obj + 0x42u, d0);
    uint16_t d1 = (uint16_t)(rd16(m, a1 + 0x10u) + sub_0D0C9E(m));
    uint16_t rnd = sub_0D0E3A(m);
    if ((int16_t)d1 > (int16_t)rnd)
        wr8(m, obj + 0x23u, (uint8_t)rd16(m, a1 + 0x12u));
    return a2;
}

/* ROM 0x1087E8 */
int Obj_1087E8_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if (rd8(m, obj + 0x22u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if (rd8(m, obj + 0x21u) == 0u)
        return 0;

    uint8_t c23 = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, c23);
    if (c23 != 0u)
        return 0;

    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 1u));
    wr8(m, obj + 0x23u, 0x14u);
    wr32(m, obj + 0x3Au, 0x012C0000u);

    uint8_t saved = rd8(m, obj + 0x23u);
    wr8(m, obj + 0x23u, 0u);
    uint32_t a2 = sub_0D09B8(m, obj, 0x001088B8u);
    if (rd8(m, obj + 0x23u) != 0u) {
        uint16_t d0 = (uint16_t)(rd16(m, a2 + 0x14u) + 0x17u);
        uint16_t d1 = (uint16_t)(rd16(m, obj + 0x3Eu) - rd16(m, a2 + 0x18u) - 8u);
        if ((int16_t)d1 < 0)
            d1 = (uint16_t)(0u - d1);
        d0 = (uint16_t)(d0 + d1 + 0x46u);
        wr16(m, obj + 0x3Au, d0);
    }
    wr8(m, obj + 0x23u, saved);

    uint32_t d0 = Obj_Spawn(m, obj, 0x001088F0u, 5u, 2u, 0u);
    if ((int32_t)d0 < 0) {
        wr8(m, obj + 0x22u, (uint8_t)(rd8(m, obj + 0x22u) - 1u));
        return 0;
    }

    uint32_t a1 = d0;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x3Eu));
    wr32(m, a1 + 0x10u, rd32(m, obj + 0x0Cu));
    wr8(m, a1 + 0x36u, rd8(m, obj + 0x20u));
    wr8(m, a1 + 0x4Eu, 0u);
    if (rd8(m, obj + 0x21u) == 0u)
        wr8(m, a1 + 0x4Eu, rd8(m, obj + 0x4Eu));
    return 0;
}
