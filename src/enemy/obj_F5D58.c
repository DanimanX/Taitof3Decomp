#include "obj_F5D58.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x006422 */

/* ROM 0x00649A */

/* ROM 0x0CBB2C */

/* ROM 0x0CEF0E */
static void sub_0CEF0E(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return;
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_ExplodeTick(m, obj);
            return;
        }
    }
    wr32(m, obj, 0x000CEF44u);
}

/* ROM 0x0D0690 */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, t);
    if (t != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0 < 0) {
        uint16_t back = rd16(m, a1);
        if (back == 0u)
            return;
        a1 -= (uint32_t)(int32_t)(int16_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
    } else {
        wr16(m, obj + 0x4Cu, d0);
    }
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D0C20 */
static uint32_t sub_0D0C20(uint32_t d0, uint32_t d1, uint16_t d2w)
{
    uint16_t d0w = (uint16_t)(d0 >> 16);
    uint16_t d1w = (uint16_t)(d1 >> 16);
    uint16_t d3w = (uint16_t)(d2w - d0w);
    if ((int16_t)d3w < 0)
        d3w = (uint16_t)(d3w + 0x0168u);

    if (d3w > 0x00B4u) {
        d3w = (uint16_t)(0x0168u - d3w);
        if (d3w <= d1w)
            return ((uint32_t)d2w << 16) | (d0 & 0xFFFFu);
        d0 -= d1;
        if ((int32_t)d0 < 0)
            d0 += 0x01680000u;
        return d0;
    } else {
        if (d3w <= d1w)
            return ((uint32_t)d2w << 16) | (d0 & 0xFFFFu);
        d0 += d1;
        if (d0 >= 0x01680000u)
            d0 -= 0x01680000u;
        return d0;
    }
}

/* ROM 0x0D0C9E */
static uint16_t sub_0D0C9E(const f3mem_t *m)
{
    uint16_t d0 = (uint16_t)(rd8(m, F3_A5 + 0x87Fu) >> 3);
    return rd16(m, 0x000D0D42u + (uint32_t)d0 * 2u);
}

/* ROM 0x0D0DC2 */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x8FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x8FEu, a1 + 4u);
    Coll_AddHazard(m, obj);
}

/* ROM 0x0D0E3A */

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

/* ROM 0x0D09B8 */
static void sub_0D09B8(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint32_t hit = rd32(m, obj + 0x68u);
    int16_t sx = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, hit));
    if (sx < 0x28 || sx > 0x127)
        return;
    int16_t sy = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, hit + 2u));
    if (sy < 0x10 || sy > 0xDF)
        return;

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
            return;
    } else {
        if ((int16_t)d7 < 0) {
            d0 = d4;
        } else {
            if (d3 > d2)
                d2 = d3;
            if (d6 > d5)
                d5 = d6;
            if (d5 >= d2)
                d0 = d4;
        }
    }

    wr16(m, obj + 0x42u, d0);
    uint16_t d1 = (uint16_t)(rd16(m, a1 + 0x10u) + sub_0D0C9E(m));
    uint16_t rnd = Obj_Next4Way(m);
    if ((int16_t)d1 > (int16_t)rnd)
        wr8(m, obj + 0x23u, (uint8_t)rd16(m, a1 + 0x12u));
}

/* ROM 0x0D1EBC */
static void sub_0D1EBC(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    int32_t idx = (int32_t)(int16_t)d0w;
    uint32_t d0 = rd32(m, 0x000D1EDCu + (uint32_t)idx * 4u);
    uint32_t d1 = rd32(m, 0x000D2044u + (uint32_t)idx * 4u);
    /* Jump table 0x0D4F26 [0..50] -> exact (D0*k, D1*k) mod 2^32 */
    *out_d0 = (uint32_t)(d0 * (uint32_t)d1w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d1w);
}

/* ROM 0x0F5FCA */
static uint16_t sub_0F5FCA(const f3mem_t *m, uint32_t obj)
{
    uint16_t base = rd16(m, obj + 0x0Cu);
    uint16_t d0w = (uint16_t)(base + 0x1Eu);
    if (d0w >= 0x0168u)
        d0w = (uint16_t)(d0w - 0x0168u);

    uint16_t d1w = (uint16_t)(base - 0x1Eu);
    if ((int16_t)d1w < 0)
        d1w = (uint16_t)(d1w + 0x0168u);

    uint16_t d2w = rd16(m, obj + 0x50u);
    if (d1w <= d0w) {
        if (d2w <= d0w && d2w >= d1w)
            return d2w;
    } else {
        if (d2w <= d0w || d2w >= d1w)
            return d2w;
    }

    uint16_t diff = (uint16_t)(d2w - base);
    if ((int16_t)diff < 0)
        diff = (uint16_t)(diff + 0x0168u);
    return (diff < 0x00B4u) ? d0w : d1w;
}

/* ROM 0x0F5D58 */
int Obj_F5D58_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    int8_t st = (int8_t)rd8(m, a1 + 0x06u);
    if (st == 0) {
        wr32(m, obj + 0x10u, 0x00063A20u);
        wr8(m, obj + 0x06u, 0u);
        uint32_t child = Obj_Spawn(m, obj, 0x000DC486u, 3u, 2u, 0u);
        if ((int32_t)child >= 0) {
            wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 7u));
            wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 7u));
            wr16(m, child + 0x58u, rd16(m, obj + 0x64u));
            wr16(m, child + 0x60u, 0x000Au);
        }
        Obj_StartDying(m, obj);
        return 0;
    }
    if (st < 0) {
        wr32(m, obj + 0x10u, 0x00063A20u);
        wr8(m, obj + 0x06u, 0u);
        sub_0CEF0E(m, obj);
        return 0;
    }

    wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 3u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 1u));
    wr32(m, obj + 0x0Cu, 0x00960000u);

    uint8_t c22 = (uint8_t)(rd8(m, obj + 0x22u) - 1u);
    wr8(m, obj + 0x22u, c22);
    if (c22 == 0u) {
        uint16_t d2w = Obj_Next4Way(m);
        d2w = (uint16_t)(d2w + (Obj_Next4Way(m) & 0x3Fu));
        d2w = (uint16_t)(d2w + (Obj_Next4Way(m) & 0x1Fu));
        wr16(m, obj + 0x50u, d2w);
        wr8(m, obj + 0x22u, 0x3Cu);
    }

    static const uint32_t speed[4] = { 0x00007530u, 0x0000EA60u, 0x00015F90u, 0x0001D4C0u };
    for (uint32_t i = 0u; i < 4u; i++) {
        uint16_t d2w = sub_0F5FCA(m, obj);
        uint32_t d0 = sub_0D0C20(rd32(m, obj + 0x58u + i * 4u), speed[i], d2w);
        wr32(m, obj + 0x58u + i * 4u, d0);
        if (i < 3u)
            wr32(m, obj + 0x0Cu, d0);
    }

    sub_0D0690(m, obj);

    uint32_t a4 = rd32(m, obj + 0x10u);
    for (uint32_t i = 0u; i < 3u; i++) {
        uint32_t dx, dy;
        sub_0D1EBC(m, rd16(m, obj + 0x58u + i * 4u), 6u, &dx, &dy);
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + dx);
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + dy);
        wr32(m, obj + 0x10u, rd32(m, a4));
        a4 += 4u;
        Obj_DrawSprite(m, obj);
    }

    uint32_t dx, dy;
    sub_0D1EBC(m, rd16(m, obj + 0x64u), 5u, &dx, &dy);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + dx - 0x00060000u);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + dy - 0x00050000u);
    wr32(m, obj + 0x10u, rd32(m, a4));
    Obj_DrawSprite(m, obj);

    a4 -= 0x0Cu;
    wr32(m, obj + 0x10u, a4);
    sub_0D0DC2(m, obj);

    uint8_t c23 = rd8(m, obj + 0x23u);
    int check_fire = 1;
    if (c23 != 0u) {
        c23 = (uint8_t)(c23 - 1u);
        wr8(m, obj + 0x23u, c23);
        if (c23 != 0u)
            check_fire = 0;
    }

    if (check_fire) {
        sub_0D09B8(m, obj, 0x000F6028u);
        if (rd8(m, obj + 0x23u) != 0u) {
            uint32_t shot = Obj_Spawn(m, obj, 0x000DA64Cu, 9u, 2u, 0u);
            if ((int32_t)shot >= 0) {
                wr16(m, shot + 0x10u, 1u);
                wr16(m, shot + 0x36u, 7u);
            }
        }
    }

    a1 = rd32(m, obj + 0x08u);
    if (rd8(m, a1 + 0x35u) == 0u && (int32_t)rd32(m, a1 + 0x24u) > 0) {
        wr8(m, a1 + 0x35u, rd8(m, obj + 0x35u));
        wr32(m, a1 + 0x36u, rd32(m, obj + 0x36u));
        wr8(m, obj + 0x35u, 0u);
        if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
            wr32(m, obj + 0x24u, 0x000F0000u);
            wr32(m, a1 + 0x24u, 0u);
        }
    }

    Obj_DrawSprite(m, a1);
    return 0;
}
