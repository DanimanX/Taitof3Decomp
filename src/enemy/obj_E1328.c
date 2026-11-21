#include "obj_E1328.h"
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

/* ROM 0x0D4A12: D0w = high word of (sin_table[idx] * k) (jump table 0x0D4E5A) */
static uint16_t sub_0D4A12(const f3mem_t *m, uint16_t k, uint16_t idx)
{
    uint32_t v = rd32(m, 0x000D4A5Au + (uint32_t)((int32_t)(int16_t)idx * 4));
    return (uint16_t)((v * (uint32_t)k) >> 16);
}

/* ROM 0x0D4A2C: D0w = hi(t*k0), D1w = hi(t*k1), t = sin_table[idx] */
static void sub_0D4A2C(const f3mem_t *m, uint16_t k0, uint16_t k1, uint16_t idx,
                       uint16_t *d0w, uint16_t *d1w)
{
    uint32_t t = rd32(m, 0x000D4A5Au + (uint32_t)((int32_t)(int16_t)idx * 4));
    *d1w = (uint16_t)((t * (uint32_t)k1) >> 16);
    *d0w = (uint16_t)((t * (uint32_t)k0) >> 16);
}

/* ROM 0x0CF7FE: arm the script when re-entering the screen */
static void sub_0CF7FE(const f3mem_t *m, uint32_t obj)
{
    uint8_t f7 = rd8(m, obj + 0x07u);
    if ((f7 & 0x80u) == 0u)
        return;
    int16_t x = (int16_t)rd16(m, obj + 0x3Au);
    if (x < -0x40 || x > 0x14F)
        return;
    f7 = (uint8_t)(f7 & ~0x80u);
    wr8(m, obj + 0x07u, f7);
    wr8(m, obj + 0x07u, (uint8_t)(f7 | 0x02u));
}

/* ROM 0x0CEF0E */
static void sub_0CEF0E(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return;
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            sub_0CBB2C(m, obj);
            return;
        }
    }
    wr32(m, obj, 0x000CEF44u);
}

/* ROM 0x0E1328 */
int Obj_E1328_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            /* 0x0E14BA */
            sub_0D0E30(m, 0x0000012Cu);
            wr8(m, obj + 0x06u, 0u);
            sub_0CEF0E(m, obj);
            return 0;
        }
    }

    /* 0x0E134C */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        sub_0D0E04(m, obj, 0x0000012Cu);
        goto lab_0e1456;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        goto lab_0e1456;

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
        if (ObjScript_CF4B8(m, obj, 0x000E08FCu) < 0)
            return -1;
    }

    /* 0x0E1388 */
    sub_0CF7FE(m, obj);
    Obj_FlagOffscreen(m, obj);
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u)
        goto lab_0e14b0;
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    if ((rd8(m, obj + 0x07u) & 0x80u) != 0u)
        return 0;
    if (rd16(m, obj + 0x60u) < 0x000Au) {
        uint16_t d0w = sub_0D4A12(m, (uint16_t)(0x000Au - rd16(m, obj + 0x60u)),
                                  rd8(m, obj + 0x1Eu));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d0w));
    }

    /* 0x0E13D4 */
    sub_0D0690(m, obj);
    {
        uint16_t d1w = rd16(m, obj + 0x68u);
        if ((int16_t)d1w >= 0) {
            uint32_t a1 = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
            if ((int32_t)a1 >= 0) {
                wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
                wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 8u));
                wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
                wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x20u));
                wr16(m, a1 + 0x58u, d1w);
                wr16(m, a1 + 0x10u, 0x0012u);
                if (rd16(m, obj + 0x58u) != 0u)
                    wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x10u));
            }
        }
    }
    sub_0CBE34(m, obj);
    if (rd8(m, obj + 0x1Eu) == 0u)
        sub_0D0DC2(m, obj);
    return 0;

lab_0e1456:;
    {
        uint32_t a1 = Obj_Spawn(m, obj, 0x000DC6D0u, 3u, 2u, 0u);
        if ((int32_t)a1 >= 0) {
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, a1 + 0x58u, 0x00B4u);
            wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));
            wr16(m, a1 + 0x10u, rd16(m, obj + 0x1Eu));
            uint16_t dx, dy;
            sub_0D4A2C(m, 0x0010u, 0x0024u, rd8(m, obj + 0x1Eu), &dx, &dy);
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + dx));
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + dy));
        }
    }

lab_0e14b0:
    wr8(m, obj + 0x06u, 0u);
    Obj_StartDying(m, obj);
    return 0;
}
