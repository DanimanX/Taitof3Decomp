#include "obj_ECB3A.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x006422 */
static uint16_t sub_006422(const f3mem_t *m, int16_t d0_in, int16_t d1_in)
{
    uint16_t d0 = (uint16_t)d0_in;
    uint16_t d1 = (uint16_t)d1_in;
    uint16_t d2 = 0u;

    if ((int16_t)d0 < 0) {
        d2 |= 2u;
        d0 = (uint16_t)(0u - d0);
    }
    if ((int16_t)d1 < 0) {
        d2 |= 4u;
        d1 = (uint16_t)(0u - d1);
    }

    uint16_t v = ((int16_t)d1 > (int16_t)d0) ? d1 : d0;
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

    if (d2 != 0u && d2 != 6u) {
        uint16_t tmp = d0;
        d0 = d1;
        d1 = tmp;
    }

    uint32_t a0 = 0x00026E40u + (uint32_t)(int32_t)(int16_t)(uint16_t)(d1 << 6);
    uint16_t b = rd8(m, a0 + (uint32_t)(int32_t)(int16_t)d0);
    return (uint16_t)((b + rd16(m, 0x00006492u + (uint32_t)(int32_t)(int16_t)d2)) & 0xFFu);
}

/* ROM 0x00649A */
static uint16_t sub_00649A(const f3mem_t *m, int16_t d0_in, int16_t d1_in)
{
    int32_t p0 = (int32_t)d0_in * (int32_t)d0_in;
    int32_t p1 = (int32_t)d1_in * (int32_t)d1_in;
    uint32_t d0 = (uint32_t)p0 + (uint32_t)p1;
    d0 += d0;

    uint16_t d1 = 4u;
    if ((int32_t)d0 >= 0x200) {
        d0 >>= 8;
        d1 = (uint16_t)(d1 + 4u);
        if ((int32_t)d0 >= 0x200) {
            d0 >>= 8;
            d1 = (uint16_t)(d1 + 4u);
            if ((int32_t)d0 >= 0x200) {
                d0 >>= 8;
                d1 = (uint16_t)(d1 + 4u);
            }
        }
    }

    uint16_t tbl = rd16(m, 0x00028340u + (uint32_t)(int32_t)(int16_t)d0 * 2u);
    d0 = (d0 & 0xFFFF0000u) | tbl;
    d0 <<= d1;
    return (uint16_t)(d0 >> 16);
}

/* ROM 0x0CBB2C */
static void sub_0CBB2C(const f3mem_t *m, uint32_t obj)
{
    wr8(m, obj + 0x32u, 0x00u);
    wr8(m, obj + 0x34u, 0xFFu);

    uint32_t a1 = rd32(m, obj + 0x10u);
    int16_t d0 = (int16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x0100u);
    int32_t prod = (int32_t)d0 * (int32_t)(int16_t)rd16(m, a1);
    a1 += 2u;
    d0 = (int16_t)((int16_t)(uint16_t)prod >> 5);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - (uint16_t)d0));

    d0 = (int16_t)((uint16_t)rd8(m, obj + 0x1Fu) - 0x0100u);
    prod = (int32_t)d0 * (int32_t)(int16_t)rd16(m, a1);
    d0 = (int16_t)((int16_t)(uint16_t)prod >> 5);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - (uint16_t)d0));

    int16_t dx = (int16_t)(rd16(m, obj + 0x14u) - 0x00A0u);
    int16_t dy = (int16_t)(rd16(m, obj + 0x18u) - 0x0074u);
    uint16_t dist = (uint16_t)(sub_00649A(m, dx, dy) << 2);
    uint16_t rad = rd16(m, F3_A5 + 0x1D5Cu);
    if ((int16_t)dist < (int16_t)rad) {
        wr32(m, obj, 0x000CBC88u);
        return;
    }
    dist = (uint16_t)(dist - rad);
    wr16(m, obj + 0x2Cu, (uint16_t)(dist << 6));

    dx = (int16_t)(rd16(m, obj + 0x14u) - 0x00A0u);
    dy = (int16_t)(rd16(m, obj + 0x18u) - 0x0074u);
    wr16(m, obj + 0x2Eu, (uint16_t)(sub_006422(m, dx, dy) << 8));
    wr16(m, obj + 0x30u, 0x0100u);

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u) {
        wr32(m, obj, 0x000CBC88u);
        return;
    }

    wr16(m, obj + 0x2Eu, (uint16_t)(rd16(m, obj + 0x2Eu) + 0x0200u));
    uint8_t s32 = rd8(m, obj + 0x32u);
    if (s32 != 0u) {
        uint32_t tbl = rd32(m, obj + 0x0Cu);
        uint16_t idx = (uint16_t)(rd16(m, obj + 0x2Eu) + 0x4800u);
        uint32_t sh = s32 & 63u;
        idx = (sh >= 16u) ? 0u : (uint16_t)(idx >> sh);
        wr32(m, obj + 0x10u, rd32(m, tbl + (uint32_t)(int32_t)(int16_t)idx * 4u));
    }

    uint16_t c2c = rd16(m, obj + 0x2Cu);
    c2c = (uint16_t)(c2c - (uint16_t)(c2c >> 5));
    wr16(m, obj + 0x2Cu, c2c);

    int32_t d1 = (int32_t)(0x0100u * (uint32_t)c2c);
    d1 >>= 8;
    d1 >>= 6;
    uint16_t r = (uint16_t)((uint16_t)d1 + rd16(m, F3_A5 + 0x1D5Cu));
    uint16_t ang = (uint16_t)(rd16(m, obj + 0x2Eu) >> 8);

    int32_t x = (int32_t)(int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)ang * 2u) * (int32_t)(int16_t)r;
    wr32(m, obj + 0x14u, (uint32_t)x + 0x00A00000u);

    int32_t y = (int32_t)(int16_t)rd16(m, 0x00027E40u + (uint32_t)ang * 2u) * (int32_t)(int16_t)r;
    wr32(m, obj + 0x18u, (uint32_t)y + 0x00740000u);

    int32_t z = (int32_t)(int16_t)(-0x0100) * (int32_t)(int16_t)rd16(m, obj + 0x30u);
    z >>= 8;
    uint8_t zb = (uint8_t)((uint16_t)z + 0x0100u);
    wr8(m, obj + 0x1Fu, zb);
    wr8(m, obj + 0x1Eu, zb);

    a1 = rd32(m, obj + 0x10u);
    d0 = (int16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x0100u);
    prod = (int32_t)d0 * (int32_t)(int16_t)rd16(m, a1);
    d0 = (int16_t)((int16_t)(uint16_t)prod >> 5);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d0));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d0));

    Obj_DrawSprite(m, obj);
    wr32(m, obj, 0x000CBC78u);
}

/* ROM 0x0CBE34 */
static void sub_0CBE34(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) != 0u) {
        if (rd8(m, obj + 0x1Du) == 0x40u) {
            wr8(m, obj + 0x35u, 0x00u);
            wr8(m, obj + 0x1Du, 0x42u);
        } else {
            wr8(m, obj + 0x1Du, 0x40u);
        }
    } else {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    }
}

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
            sub_0CBB2C(m, obj);
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

/* ROM 0x0D005A */
static void sub_0D005A(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d0)
{
    int16_t off = (int16_t)rd16(m, 0x000D00B4u + (uint32_t)(int32_t)(int16_t)d0 * 2u);
    a1 = rd32(m, a1 + (uint32_t)(int32_t)off);
    uint32_t a2 = rd32(m, obj + 0x0Cu);
    if (a1 == rd32(m, a2 + 0x28u))
        return;

    wr32(m, obj + 0x48u, a1);
    wr32(m, a2 + 0x28u, a1);
    wr16(m, obj + 0x4Cu, rd16(m, a1));
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
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
static void sub_0D0E30(const f3mem_t *m, uint32_t d0)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
}

/* ROM 0x0D2D44: D0 = rd32(0xD2D64 + a*4) * k, D1 = rd32(0xD2ECC + a*4) * k
 * (jump table 0x0D4F26 is an exact multiply for k = 0..50) */
static void sub_0D2D44(const f3mem_t *m, uint16_t a, uint16_t k,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t i = (uint32_t)((int32_t)(int16_t)a * 4);
    *out_d0 = rd32(m, 0x000D2D64u + i) * (uint32_t)k;
    *out_d1 = rd32(m, 0x000D2ECCu + i) * (uint32_t)k;
}

/* ROM 0x0ECD66 (state 0) / 0x0ECDAA (state 1): arc movement */
static void sub_0ECD66(const f3mem_t *m, uint32_t obj, int state1)
{
    uint16_t ang;
    if (!state1) {
        int32_t r = (int32_t)(rd32(m, obj + 0x60u) - rd32(m, obj + 0x64u));
        wr32(m, obj + 0x60u, (uint32_t)r);
        if (r < 0) {
            wr32(m, obj + 0x60u, 0u);
            wr8(m, obj + 0x06u, 1u);
            return;
        }
        ang = 0x005Au;
    } else {
        wr32(m, obj + 0x60u, rd32(m, obj + 0x60u) + rd32(m, obj + 0x64u));
        ang = 0x010Eu;
    }
    uint32_t d0, d1;
    sub_0D2D44(m, ang, rd16(m, obj + 0x60u), &d0, &d1);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - rd32(m, obj + 0x50u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d1);
    uint16_t d0w = rd16(m, obj + 0x50u);
    uint16_t d1w = (uint16_t)(0u - (uint16_t)(d1 >> 16));
    wr16(m, obj + 0x58u, sub_0D0E52(m, (int16_t)d0w, (int16_t)d1w));
}

/* ROM 0x0ECB3A */
int Obj_ECB3A_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0;
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        sub_0D0E04(m, obj, 0x000000C8u);
        goto lab_0ecc20;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        goto lab_0ecc20;
    if ((int16_t)rd16(m, obj + 0x18u) > (int16_t)rd16(m, obj + 0x3Eu))
        goto lab_0eccb6;

    {
        uint8_t st = rd8(m, obj + 0x06u);
        if (st > 1u)
            return -1;   /* outside the 2-entry jump table 0x0ECD5E */
        uint32_t s = rd32(m, F3_A5 + 0x4228u);
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - s);
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - s);
        sub_0ECD66(m, obj, st == 1u);
    }

    /* 0x0ECB72 */
    if (rd8(m, obj + 0x21u) != 0u) {
        uint8_t c23 = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, c23);
        if (c23 == 0u) {
            wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 1u));
            wr8(m, obj + 0x23u, 3u);
            uint32_t a1 = Obj_Spawn(m, obj, 0x000ECFAEu, 5u, 2u, 0u);
            if ((int32_t)a1 >= 0) {
                wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
                wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
                wr32(m, a1 + 0x10u, rd32(m, obj + 0x3Eu));
                wr8(m, a1 + 0x36u, rd8(m, obj + 0x21u));
            }
        }
    }

    /* 0x0ECBC8 */
    sub_0D005A(m, obj, 0x000ECE18u, rd16(m, obj + 0x58u));
    sub_0D0690(m, obj);
    sub_0D0DC2(m, obj);
    sub_0CBE34(m, obj);
    Obj_DrawSprite(m, obj);
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            /* 0x0ECD00 */
            d0 = 0x000000C8u;
            sub_0D0E30(m, d0);
            uint32_t parent = rd32(m, obj + 0x08u);
            uint8_t rem = (uint8_t)(rd8(m, parent + 0x22u) - 1u);
            wr8(m, parent + 0x22u, rem);
            if (rem != 0u || (rd8(m, parent + 0x07u) & 0x01u) != 0u) {
                sub_0CEE9A(m, obj, d0);
                return 0;
            }
            if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
                Obj_StartDying(m, obj);
                return 0;
            }
            sub_0D0E30(m, 0x000007D0u);
            d0 = Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x000007D0u);
            sub_0CEEA2(m, obj, d0);
        }
    }
    return 0;

lab_0ecc20:
    d0 = Obj_Spawn(m, obj, 0x000DC586u, 3u, 2u, 0u);
    if ((int32_t)d0 >= 0) {
        wr32(m, d0 + 0x14u, rd32(m, obj + 0x14u));
        wr16(m, d0 + 0x14u, (uint16_t)(rd16(m, d0 + 0x14u) + 8u));
        wr32(m, d0 + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, d0 + 0x18u, (uint16_t)(rd16(m, d0 + 0x18u) + 8u));
        wr16(m, d0 + 0x58u, rd16(m, obj + 0x58u));
        wr16(m, d0 + 0x60u, rd16(m, obj + 0x60u));
    }
    {
        /* 0x0ECC64 */
        uint32_t parent = rd32(m, obj + 0x08u);
        uint8_t rem = (uint8_t)(rd8(m, parent + 0x22u) - 1u);
        wr8(m, parent + 0x22u, rem);
        if (rem != 0u || (rd8(m, parent + 0x07u) & 0x01u) != 0u) {
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

lab_0eccb6:
    {
        uint32_t a1 = Obj_Spawn(m, obj, 0x000ED124u, 5u, 2u, 0u);
        if ((int32_t)a1 >= 0) {
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) - 0x10u));
        }
        uint32_t parent = rd32(m, obj + 0x08u);
        wr8(m, parent + 0x22u, (uint8_t)(rd8(m, parent + 0x22u) - 1u));
        wr8(m, parent + 0x07u, (uint8_t)(rd8(m, parent + 0x07u) | 0x01u));
        Obj_StartDying(m, obj);
        return 0;
    }
}
