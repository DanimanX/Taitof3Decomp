#include "obj_108E8E.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x006422 */

/* ROM 0x00649A */

/* ROM 0x0CBB2C */

/* ROM 0x0CBE34 */

/* ROM 0x0CEE52 */
static void sub_0CEE52(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t b = rd8(m, obj + 0x4Eu);
    if (b != 0u) {
        d0 = (d0 & 0xFFFFFF00u) | b;
        Obj_Spawn(m, obj, 0x00117C28u, 0u, 1u, d0);
    }
    Obj_StartDying(m, obj);
}

/* ROM 0x0CEE48 */
static void sub_0CEE48(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return;
    }
    sub_0CEE52(m, obj, d0);
}

/* ROM 0x0CEEA2 */
static void sub_0CEEA2(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t b = rd8(m, obj + 0x4Eu);
    if (b != 0u) {
        d0 = (d0 & 0xFFFFFF00u) | b;
        Obj_Spawn(m, obj, 0x00117C28u, 0u, 1u, d0);
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_ExplodeTick(m, obj);
            return;
        }
    }
    wr32(m, obj, 0x000CEEF0u);
}

/* ROM 0x0CEE9A */
static void sub_0CEE9A(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return;
    }
    sub_0CEEA2(m, obj, d0);
}

/* ROM 0x0D0C9E */
static uint16_t sub_0D0C9E(const f3mem_t *m)
{
    uint16_t d0 = (uint16_t)(rd8(m, F3_A5 + 0x87Fu) >> 3);
    return rd16(m, 0x000D0D42u + (uint32_t)d0 * 2u);
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
/* ROM 0x0D0DC2 */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x8FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x8FEu, a1 + 4u);
    Coll_AddHazard(m, obj);
}
/* ROM 0x0D0E04 */
static void sub_0D0E04(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t hit = rd8(m, obj + 0x35u);
    if (hit == 1u) {
        uint32_t a1 = rd32(m, rd32(m, obj + 0x36u) + 0x08u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    } else if (hit == 2u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    }
}
/* ROM 0x0D0E30 */

/* ROM 0x0CFA7C: frame record A1 + table 0x0CFA96[d0] -> sprite + hit box */
static void sub_0CFA7C(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d0w)
{
    uint16_t off = rd16(m, 0x000CFA96u + (uint32_t)((int32_t)(int16_t)d0w * 2));
    a1 += (uint32_t)(int32_t)(int16_t)off;
    /* 0x0CFA86 */
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D06E0: +0x42 = angle to the nearer active player (0xB4 if none) */
static void sub_0D06E0(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x42u, 0x00B4u);
    uint16_t d2 = 0u, d3 = 0u, d4 = 0xFFFFu, d5 = 0u, d6 = 0u, d7 = 0xFFFFu;
    uint32_t p = 0x0041B190u;
    if ((int16_t)rd16(m, p + 0x4Cu) >= 0) {
        uint32_t a2 = rd32(m, obj + 0x68u);
        uint16_t dx = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, p + 0x14u) + rd16(m, a2) - 0x17u);
        d2 = ((int16_t)dx < 0) ? (uint16_t)(0u - dx) : dx;
        uint16_t dy = (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, p + 0x18u) + rd16(m, a2 + 2u) - 0x0Fu);
        d3 = ((int16_t)dy < 0) ? (uint16_t)(0u - dy) : dy;
        d4 = sub_0D0E52(m, (int16_t)dx, (int16_t)dy);
    }
    p = 0x0041B21Au;
    if ((int16_t)rd16(m, p + 0x4Cu) >= 0) {
        uint32_t a2 = rd32(m, obj + 0x68u);
        uint16_t dx = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, p + 0x14u) + rd16(m, a2) - 0x17u);
        d5 = ((int16_t)dx < 0) ? (uint16_t)(0u - dx) : dx;
        uint16_t dy = (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, p + 0x18u) + rd16(m, a2 + 2u) - 0x0Fu);
        d6 = ((int16_t)dy < 0) ? (uint16_t)(0u - dy) : dy;
        d7 = sub_0D0E52(m, (int16_t)dx, (int16_t)dy);
    }
    if ((int16_t)d4 < 0) {
        if ((int16_t)d7 >= 0)
            wr16(m, obj + 0x42u, d7);
        return;
    }
    if ((int16_t)d7 < 0) {
        wr16(m, obj + 0x42u, d4);
        return;
    }
    if (d3 > d2)
        d2 = d3;
    if (d6 > d5)
        d5 = d6;
    wr16(m, obj + 0x42u, (d5 < d2) ? d7 : d4);
}

/* 0x1090DC / 0x10919E: two aimed shots (0xD9092) from the offsets at +0x68 */
static void fire_pair(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x68u);
    uint16_t d5 = rd16(m, a4); a4 += 2u;
    uint32_t d6 = rd32(m, obj + 0x14u), d7 = rd32(m, obj + 0x18u);
    for (int i = 0; i < 2; i++) {
        uint32_t a1 = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
        if ((int32_t)a1 < 0)
            return;
        wr32(m, a1 + 0x14u, d6);
        wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + rd16(m, a4))); a4 += 2u;
        wr32(m, a1 + 0x18u, d7);
        wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + rd16(m, a4)));
        if (i == 0) a4 += 2u;
        wr16(m, a1 + 0x58u, d5);
        wr16(m, a1 + 0x10u, 0u);
    }
}

/* 0x1090BA / 0x10917C: aim with hit box 0x10947A, then fire */
static void aim_and_fire(const f3mem_t *m, uint32_t obj)
{
    uint32_t saved = rd32(m, obj + 0x68u);
    wr32(m, obj + 0x68u, 0x0010947Au);
    sub_0D09B8(m, obj, 0x00109306u);
    wr32(m, obj + 0x68u, saved);
    if (rd8(m, obj + 0x23u) == 0u)
        return;
    fire_pair(m, obj);
}

/* ROM 0x109068: state 0 */
static void sub_109068(const f3mem_t *m, uint32_t obj)
{
    uint8_t s23 = rd8(m, obj + 0x23u);
    uint32_t s68 = rd32(m, obj + 0x68u);
    wr32(m, obj + 0x68u, 0x0010947Au);
    wr8(m, obj + 0x23u, 0u);
    sub_0D09B8(m, obj, 0x001092CEu);
    wr32(m, obj + 0x68u, s68);
    if (rd8(m, obj + 0x23u) != 0u) {
        wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
        wr8(m, obj + 0x06u, 1u);
    }
    wr8(m, obj + 0x23u, s23);
    if (rd8(m, obj + 0x22u) != 0u)
        return;
    if (s23 != 0u) {
        s23 = (uint8_t)(s23 - 1u);
        wr8(m, obj + 0x23u, s23);
        if (s23 != 0u)
            return;
    }
    aim_and_fire(m, obj);
}

/* ROM 0x10916C: state 1 */
static void sub_10916C(const f3mem_t *m, uint32_t obj)
{
    uint8_t c23 = rd8(m, obj + 0x23u);
    if (c23 != 0u) {
        c23 = (uint8_t)(c23 - 1u);
        wr8(m, obj + 0x23u, c23);
        if (c23 != 0u)
            return;
    }
    aim_and_fire(m, obj);
}

/* ROM 0x108E8E */
int Obj_108E8E_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0;
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        sub_0D0E04(m, obj, 0x000000C8u);
        goto lab_108f54;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        goto lab_108f54;

    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
    {
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
        if ((f7 & 0x02u) != 0u) {
            if (ObjScript_CF1E4(m, obj) < 0)
                return -1;
        }
    }
    if (rd8(m, obj + 0x20u) != 0u) {
        if (ObjScript_CF4B8(m, obj, 0x001092CEu) < 0)
            return -1;
    }

    /* 0x108ECA */
    Obj_FlagOffscreen(m, obj);
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr8(m, a1 + 0x22u, (uint8_t)(rd8(m, a1 + 0x22u) - 1u));
        wr8(m, a1 + 0x07u, (uint8_t)(rd8(m, a1 + 0x07u) | 0x01u));
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x108EDA: state jump table 0x109060 */
    {
        uint8_t st = rd8(m, obj + 0x06u);
        if (st > 1u)
            return -1;
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
        if (st == 0u)
            sub_109068(m, obj);
        else
            sub_10916C(m, obj);
    }

    /* 0x108EF4 */
    wr32(m, obj + 0x68u, 0x0010947Au);
    sub_0D06E0(m, obj);
    sub_0CFA7C(m, obj, 0x001093DAu, rd16(m, obj + 0x42u));
    sub_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            /* 0x109002 */
            d0 = 0x000000C8u;
            Obj_RelayTimer(m, d0);
            uint32_t a1 = rd32(m, obj + 0x08u);
            uint8_t rem = (uint8_t)(rd8(m, a1 + 0x22u) - 1u);
            wr8(m, a1 + 0x22u, rem);
            if (rem != 0u || (rd8(m, a1 + 0x07u) & 0x01u) != 0u) {
                sub_0CEE9A(m, obj, d0);
                return 0;
            }
            if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
                Obj_StartDying(m, obj);
                return 0;
            }
            Obj_RelayTimer(m, 0x000007D0u);
            d0 = Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x000007D0u);
            sub_0CEEA2(m, obj, d0);
        }
    }
    return 0;

lab_108f54:
    d0 = Obj_Spawn(m, obj, 0x000DC486u, 3u, 2u, 0u);
    if ((int32_t)d0 >= 0) {
        wr32(m, d0 + 0x14u, rd32(m, obj + 0x14u));
        wr16(m, d0 + 0x14u, (uint16_t)(rd16(m, d0 + 0x14u) + 0x10u));
        wr32(m, d0 + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, d0 + 0x18u, (uint16_t)(rd16(m, d0 + 0x18u) + 0x10u));
        wr16(m, d0 + 0x58u, rd16(m, obj + 0x58u));
        wr16(m, d0 + 0x60u, rd16(m, obj + 0x60u));
    }
    {
        /* 0x108F9C */
        uint32_t a1 = rd32(m, obj + 0x08u);
        uint8_t rem = (uint8_t)(rd8(m, a1 + 0x22u) - 1u);
        wr8(m, a1 + 0x22u, rem);
        if (rem != 0u || (rd8(m, a1 + 0x07u) & 0x01u) != 0u) {
            sub_0CEE48(m, obj, d0);
            return 0;
        }
        if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
            Obj_StartDying(m, obj);
            return 0;
        }
        sub_0D0E04(m, obj, 0x000007D0u);
        d0 = Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x000007D0u);
        sub_0CEE52(m, obj, d0);
        return 0;
    }
}
