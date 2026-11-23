#include "obj_F0846.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

#define ASR16(v, n) ((int16_t)(((int16_t)(v) < 0) \
    ? ~(uint16_t)((uint16_t)~(uint16_t)(v) >> (n)) \
    : ((uint16_t)(v) >> (n))))

#define ASR32(v, n) ((int32_t)(((int32_t)(v) < 0) \
    ? ~(uint32_t)((uint32_t)~(uint32_t)(v) >> (n)) \
    : ((uint32_t)(v) >> (n))))

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

/* ROM 0x0CF7FE */
static void sub_0CF7FE(const f3mem_t *m, uint32_t obj)
{
    uint8_t flags = rd8(m, obj + 0x07u);
    if ((flags & 0x80u) == 0u)
        return;
    int16_t x = (int16_t)rd16(m, obj + 0x3Au);
    if (x < -0x40 || x > 0x014F)
        return;
    wr8(m, obj + 0x07u, (uint8_t)((flags & (uint8_t)~0x80u) | 0x02u));
}

/* ROM 0x0D0678: script pointer + first frame */
static void sub_0D0678(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x48u, a1);
    wr16(m, obj + 0x4Cu, rd16(m, a1)); a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
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

/* ROM 0x0D06E0: aim angle toward nearest active player */
static void sub_0D06E0(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x42u, 0x00B4u);

    uint16_t d2 = 0u, d3 = 0u, d4 = 0xFFFFu;
    if ((int16_t)rd16(m, 0x41B190u + 0x4Cu) >= 0) {
        uint32_t a2 = rd32(m, obj + 0x68u);
        d2 = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, 0x41B190u + 0x14u) + rd16(m, a2) - 0x17u);
        uint16_t a3w = d2;
        if ((int16_t)d2 < 0)
            d2 = (uint16_t)(0u - d2);
        d3 = (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, 0x41B190u + 0x18u) + rd16(m, a2 + 2u) - 0x0Fu);
        uint16_t a2w = d3;
        if ((int16_t)d3 < 0)
            d3 = (uint16_t)(0u - d3);
        d4 = sub_0D0E52(m, (int16_t)a3w, (int16_t)a2w);
    }

    uint16_t d5 = 0u, d6 = 0u, d7 = 0xFFFFu;
    if ((int16_t)rd16(m, 0x41B21Au + 0x4Cu) >= 0) {
        uint32_t a2 = rd32(m, obj + 0x68u);
        d5 = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, 0x41B21Au + 0x14u) + rd16(m, a2) - 0x17u);
        uint16_t a3w = d5;
        if ((int16_t)d5 < 0)
            d5 = (uint16_t)(0u - d5);
        d6 = (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, 0x41B21Au + 0x18u) + rd16(m, a2 + 2u) - 0x0Fu);
        uint16_t a2w = d6;
        if ((int16_t)d6 < 0)
            d6 = (uint16_t)(0u - d6);
        d7 = sub_0D0E52(m, (int16_t)a3w, (int16_t)a2w);
    }

    if ((int16_t)d4 < 0) {
        if ((int16_t)d7 >= 0)
            wr16(m, obj + 0x42u, d7);
        return;
    }

    if ((int16_t)d7 >= 0) {
        if (d3 > d2)
            d2 = d3;
        if (d6 > d5)
            d5 = d6;
        if (d5 < d2) {
            wr16(m, obj + 0x42u, d7);
            return;
        }
    }
    wr16(m, obj + 0x42u, d4);
}

/* Helper for one player check inside ROM 0x0D09B8 */
static uint16_t check_player_0D09B8(const f3mem_t *m, uint32_t obj, uint32_t a1,
                                    uint32_t player, uint16_t *out_dx, uint16_t *out_dy)
{
    if ((int16_t)rd16(m, player + 0x4Cu) < 0)
        return 0xFFFFu;

    uint32_t a4 = rd32(m, obj + 0x68u) + 2u;
    uint16_t dy = (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, player + 0x18u) + rd16(m, a4) - 0x0Fu);
    uint16_t a3w = dy;
    if ((int16_t)dy < 0)
        dy = (uint16_t)(0u - dy);
    if (dy > rd16(m, a1 + 0x0Au)) {
        *out_dy = dy;
        return 0xFFFFu;
    }

    uint16_t dx = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, player + 0x14u) + rd16(m, a4 - 2u) - 0x17u);
    uint16_t a4w = dx;
    if ((int16_t)dx < 0) {
        dx = (uint16_t)(0u - dx);
        if (dx > rd16(m, a1 + 0x0Au) ||
            (dx < rd16(m, a1 + 0x08u) && dy < rd16(m, a1 + 0x08u))) {
            *out_dx = dx;
            *out_dy = dy;
            return 0xFFFFu;
        }
    } else {
        if (dx > rd16(m, a1 + 0x0Au)) {
            *out_dx = dx;
            *out_dy = dy;
            return 0xFFFFu;
        }
        int16_t thresh = (int16_t)(rd16(m, a1 + 0x08u) - 0x1Eu);
        if ((int16_t)dx < thresh && dy < rd16(m, a1 + 0x08u)) {
            *out_dx = dx;
            *out_dy = dy;
            return 0xFFFFu;
        }
    }

    *out_dx = dx;
    *out_dy = dy;
    uint16_t ang = sub_0D0E52(m, (int16_t)a4w, (int16_t)a3w);
    uint16_t amin = rd16(m, a1 + 0x0Cu);
    uint16_t amax = rd16(m, a1 + 0x0Eu);
    if (amin > amax)
        return (ang >= amin || ang <= amax) ? ang : 0xFFFFu;
    return (ang >= amin && ang <= amax) ? ang : 0xFFFFu;
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

    uint16_t d2 = 0u, d3 = 0u;
    uint16_t d4 = check_player_0D09B8(m, obj, a1, 0x41B190u, &d2, &d3);

    uint16_t d5 = 0u, d6 = 0u;
    uint16_t d7 = check_player_0D09B8(m, obj, a1, 0x41B21Au, &d5, &d6);

    uint16_t d0 = d7;
    if ((int16_t)d4 < 0) {
        if ((int16_t)d7 < 0)
            return;
    } else if ((int16_t)d7 < 0) {
        d0 = d4;
    } else {
        if (d3 > d2)
            d2 = d3;
        if (d6 > d5)
            d5 = d6;
        if (d5 >= d2)
            d0 = d4;
    }

    wr16(m, obj + 0x42u, d0);
    uint16_t d1 = (uint16_t)(rd16(m, a1 + 0x10u) + sub_0D0C9E(m));
    uint16_t rnd = Obj_Next4Way(m);
    if ((int16_t)d1 > (int16_t)rnd)
        wr8(m, obj + 0x23u, (uint8_t)rd16(m, a1 + 0x12u));
}

/* ROM 0x0F0846 */
int Obj_F0846_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));

            /* 0x0F09AE */
            Obj_RelayTimer(m, 0xC8u);
            uint32_t parent = rd32(m, obj + 0x08u);
            uint8_t p22 = (uint8_t)(rd8(m, parent + 0x22u) - 1u);
            wr8(m, parent + 0x22u, p22);
            if (p22 != 0u || (rd8(m, parent + 0x07u) & 0x01u) != 0u) {
                sub_0CEE9A(m, obj, 0xC8u);
                return 0;
            }
            if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
                Obj_StartDying(m, obj);
                return 0;
            }
            Obj_RelayTimer(m, 0x7D0u);
            uint32_t score_child = Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x7D0u);
            sub_0CEEA2(m, obj, score_child);
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        sub_0D0E04(m, obj, 0xC8u);
        goto death_spawn;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        goto death_spawn;

    /* 0x0F087C */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
    if (rd8(m, obj + 0x06u) != 3u) {
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & (uint8_t)~0x02u));
        if ((f7 & 0x02u) != 0u) {
            if (ObjScript_CF1E4(m, obj) < 0)
                return -1;
        }
        if (rd8(m, obj + 0x20u) != 0u) {
            if (ObjScript_CF4B8(m, obj, 0x000F0B32u) < 0)
                return -1;
        }
    }

    /* 0x0F08B0 */
    Obj_FlagOffscreen(m, obj);
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        uint32_t parent = rd32(m, obj + 0x08u);
        wr8(m, parent + 0x22u, (uint8_t)(rd8(m, parent + 0x22u) - 1u));
        wr8(m, parent + 0x07u, (uint8_t)(rd8(m, parent + 0x07u) | 0x01u));
        Obj_StartDying(m, obj);
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    switch (rd8(m, obj + 0x06u)) {
    case 0u: /* 0x0F0A20 */
        sub_0CF7FE(m, obj);
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0u)
            wr8(m, obj + 0x06u, 1u);
        break;

    case 1u: /* 0x0F0A38 */
        if (rd8(m, obj + 0x23u) == 0u) {
            if ((rd8(m, obj + 0x07u) & 0x20u) == 0u)
                break;
            wr32(m, obj + 0x0Cu, rd32(m, obj + 0x68u));
            wr32(m, obj + 0x68u, 0x000F0D20u);
            sub_0D09B8(m, obj, 0x000F0B32u);
            wr32(m, obj + 0x68u, rd32(m, obj + 0x0Cu));
            if (rd8(m, obj + 0x23u) == 0u)
                break;
        }
        {
            uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
            wr8(m, obj + 0x23u, t);
            if (t == 0u)
                wr8(m, obj + 0x06u, 2u);
        }
        break;

    case 2u: /* 0x0F0A80 */
        if (rd16(m, obj + 0x68u) != 0u) {
            sub_0D0678(m, obj, 0x000F0C24u);
            wr8(m, obj + 0x06u, 3u);
        }
        break;

    case 3u: /* 0x0F0A9A */
        if (rd16(m, obj + 0x4Cu) == 0u) {
            sub_0D0678(m, obj, 0x000F0BA0u);
            wr8(m, obj + 0x23u, 0x1Eu);
            wr8(m, obj + 0x06u, 4u);
            break;
        }
        if (rd16(m, obj + 0x68u) == 0u)
            break;
        wr32(m, obj + 0x0Cu, rd32(m, obj + 0x68u));
        wr32(m, obj + 0x68u, 0x000F0D20u);
        sub_0D06E0(m, obj);
        wr32(m, obj + 0x68u, rd32(m, obj + 0x0Cu));
        {
            uint32_t shot = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
            if ((int32_t)shot >= 0) {
                wr32(m, shot + 0x14u, rd32(m, obj + 0x14u));
                wr16(m, shot + 0x14u, (uint16_t)(rd16(m, shot + 0x14u) + 8u));
                wr32(m, shot + 0x18u, rd32(m, obj + 0x18u));
                wr16(m, shot + 0x18u, (uint16_t)(rd16(m, shot + 0x18u) + 0x10u));
                wr16(m, shot + 0x58u, rd16(m, obj + 0x42u));
                wr16(m, shot + 0x10u, 0u);
            }
        }
        break;

    case 4u: /* 0x0F0B22 */
        {
            uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
            wr8(m, obj + 0x23u, t);
            if (t == 0u)
                wr8(m, obj + 0x06u, 1u);
        }
        break;

    default:
        return -1;
    }

    /* 0x0F08DA */
    sub_0D0690(m, obj);
    sub_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);
    return 0;

death_spawn:
    {
        uint32_t child = Obj_Spawn(m, obj, 0x000DC486u, 3u, 2u, 0u);
        if ((int32_t)child >= 0) {
            wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x10u));
            wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x10u));
            wr16(m, child + 0x58u, rd16(m, obj + 0x58u));
            wr16(m, child + 0x60u, rd16(m, obj + 0x60u));
        }

        uint32_t parent = rd32(m, obj + 0x08u);
        uint8_t p22 = (uint8_t)(rd8(m, parent + 0x22u) - 1u);
        wr8(m, parent + 0x22u, p22);
        if (p22 != 0u || (rd8(m, parent + 0x07u) & 0x01u) != 0u) {
            sub_0CEE48(m, obj, child);
            return 0;
        }
        if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
            Obj_StartDying(m, obj);
            return 0;
        }
        sub_0D0E04(m, obj, 0x7D0u);
        uint32_t score_child = Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x7D0u);
        sub_0CEE52(m, obj, score_child);
        return 0;
    }
}
