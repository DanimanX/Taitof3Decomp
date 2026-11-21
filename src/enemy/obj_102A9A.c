#include "obj_102A9A.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x0CFA86 */
static void sub_0CFA86(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)d0);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D0C9E */
static uint16_t sub_0D0C9E(const f3mem_t *m)
{
    uint16_t d0 = (uint16_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
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
static uint16_t sub_0D0E52(const f3mem_t *m, uint16_t d0, uint16_t d1)
{
    uint16_t d2 = 0u;

    if ((int16_t)d0 < 0) {
        d2 = (uint16_t)(d2 + 2u);
        d0 = (uint16_t)(0u - d0);
    }
    if ((int16_t)d1 < 0) {
        d2 = (uint16_t)(d2 + 4u);
        d1 = (uint16_t)(0u - d1);
    }

    uint16_t src = (d1 > d0) ? d1 : d0;
    uint16_t d3 = 0x20u;
    for (uint16_t i = 0u; i < 16u; i++) {
        if ((src & (uint16_t)(0x8000u >> i)) != 0u) {
            d3 = (uint16_t)(0x10u + i);
            break;
        }
    }

    d3 = (uint16_t)(d3 - 0x1Au);
    if ((int16_t)d3 < 0) {
        uint16_t shift = (uint16_t)(0u - d3) & 63u;
        d0 = (uint16_t)(d0 >> shift);
        d1 = (uint16_t)(d1 >> shift);
    }

    if (d2 != 2u && d2 != 4u) {
        uint16_t tmp = d0;
        d0 = d1;
        d1 = tmp;
    }

    d1 = (uint16_t)((uint16_t)(d1 << 6) + d0);
    uint16_t res = (uint16_t)rd8(m, 0x000D0EBCu + (uint32_t)(int32_t)(int16_t)d1);
    res = (uint16_t)(res + rd16(m, 0x000D0EB4u + (uint32_t)(int32_t)(int16_t)d2));
    if (res == 0x0168u)
        res = 0u;
    return res;
}

/* ROM 0x0D09B8 */
static void sub_0D09B8(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint32_t a2 = rd32(m, obj + 0x68u);
    int16_t mx = (int16_t)(uint16_t)(rd16(m, obj + 0x14u) + rd16(m, a2));
    if (mx < 0x0028 || mx > 0x0127)
        return;
    int16_t my = (int16_t)(uint16_t)(rd16(m, obj + 0x18u) + rd16(m, a2 + 2u));
    if (my < 0x0010 || my > 0x00DF)
        return;

    uint16_t adx[2] = { 0u, 0u };
    uint16_t ady[2] = { 0u, 0u };
    int16_t ang[2] = { -1, -1 };

    for (int p = 0; p < 2; p++) {
        uint32_t pl = (p == 0) ? 0x0041B190u : 0x0041B21Au;
        if ((int16_t)rd16(m, pl + 0x4Cu) < 0)
            continue;

        uint32_t a4 = rd32(m, obj + 0x68u) + 2u;
        uint16_t dy = (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, pl + 0x18u)
                                 + rd16(m, a4) - 0x000Fu);
        uint16_t a3w = dy;
        if ((int16_t)dy < 0)
            dy = (uint16_t)(0u - dy);
        ady[p] = dy;
        if (dy > rd16(m, a1 + 0x0Au))
            continue;

        a4 -= 2u;
        uint16_t dx = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, pl + 0x14u)
                                 + rd16(m, a4) - 0x0017u);
        uint16_t a4w = dx;
        uint16_t near = rd16(m, a1 + 0x08u);
        if ((int16_t)dx < 0) {
            dx = (uint16_t)(0u - dx);
            adx[p] = dx;
            if (dx > rd16(m, a1 + 0x0Au))
                continue;
            if (dx < near && dy < near)
                continue;
        } else {
            adx[p] = dx;
            if (dx > rd16(m, a1 + 0x0Au))
                continue;
            uint16_t thresh = (uint16_t)(near - 0x001Eu);
            if ((int16_t)dx < (int16_t)thresh && dy < near)
                continue;
        }

        uint16_t a = sub_0D0E52(m, a4w, a3w);
        uint16_t lo = rd16(m, a1 + 0x0Cu);
        uint16_t hi = rd16(m, a1 + 0x0Eu);
        if (lo > hi) {
            if (!(a >= lo || a <= hi))
                continue;
        } else {
            if (a < lo || a > hi)
                continue;
        }
        ang[p] = (int16_t)a;
    }

    int16_t d0 = ang[1];
    if (ang[0] < 0) {
        if (ang[1] < 0)
            return;
    } else if (ang[1] < 0) {
        d0 = ang[0];
    } else {
        if (ady[0] > adx[0])
            adx[0] = ady[0];
        if (ady[1] > adx[1])
            adx[1] = ady[1];
        if (adx[1] >= adx[0])
            d0 = ang[0];
    }

    wr16(m, obj + 0x42u, (uint16_t)d0);
    uint16_t d1 = (uint16_t)(rd16(m, a1 + 0x10u) + sub_0D0C9E(m));
    uint16_t roll = sub_0D0E3A(m);
    if ((int16_t)d1 > (int16_t)roll)
        wr8(m, obj + 0x23u, (uint8_t)rd16(m, a1 + 0x12u));
}

/* ROM 0x102A9A */
int Obj_102A9A_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x3Au, rd32(m, a1 + 0x3Au));
    wr32(m, obj + 0x3Eu, rd32(m, a1 + 0x3Eu));
    wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));
    uint8_t b22 = rd8(m, a1 + 0x22u);
    wr8(m, obj + 0x22u, b22);
    if (b22 == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    a1 = rd32(m, obj + 0x0Cu);
    for (int i = 0; i < 10; i++) {
        uint8_t t0 = (uint8_t)(rd8(m, a1) - 1u);
        wr8(m, a1, t0);
        if (t0 == 0u) {
            wr8(m, a1, 0x0Fu);
            uint8_t f0 = (uint8_t)(rd8(m, a1 + 1u) + 1u);
            wr8(m, a1 + 1u, f0);
            if (f0 > 0x0Bu)
                wr8(m, a1 + 1u, 0u);
        }
        a1 += 2u;

        uint8_t t1 = (uint8_t)(rd8(m, a1) - 1u);
        wr8(m, a1, t1);
        if (t1 == 0u) {
            wr8(m, a1, 0x0Au);
            uint8_t f1 = (uint8_t)(rd8(m, a1 + 1u) + 1u);
            wr8(m, a1 + 1u, f1);
            if (f1 > 0x0Du)
                wr8(m, a1 + 1u, 0u);
        }
        a1 += 2u;
    }

    uint8_t cd = rd8(m, obj + 0x23u);
    if (cd != 0u)
        wr8(m, obj + 0x23u, (uint8_t)(cd - 1u));

    uint32_t p1 = rd32(m, obj + 0x08u);
    uint32_t p2 = rd32(m, p1 + 0x08u);
    uint32_t p3 = rd32(m, p2 + 0x08u);
    uint32_t p4 = rd32(m, p3 + 0x08u);
    Obj_DrawSprite(m, p4);
    Obj_DrawSprite(m, p3);
    Obj_DrawSprite(m, p2);

    uint32_t a4 = rd32(m, obj + 0x0Cu);
    for (int16_t d7 = 9; d7 >= 0; d7--) {
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

        uint16_t d0 = (uint16_t)(rd16(m, a4) & 0x00FFu);
        a4 += 2u;
        wr16(m, obj + 0x44u, d0);
        uint32_t rec1 = (d7 < 5)
            ? rd32(m, 0x00102DE2u + (uint32_t)d0 * 4u)
            : rd32(m, 0x00102D7Au + (uint32_t)d0 * 4u);
        sub_0CFA86(m, obj, rec1);

        uint32_t off1 = 0x00102CD6u + (uint32_t)(int32_t)d7 * 4u;
        uint16_t dx1 = (uint16_t)(rd16(m, off1) + rd16(m, obj + 0x68u));
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + dx1));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, off1 + 2u)));
        Obj_DrawSprite(m, obj);

        d0 = (uint16_t)(rd16(m, a4) & 0x00FFu);
        a4 += 2u;
        wr16(m, obj + 0x46u, d0);
        uint32_t rec2 = (d7 < 5)
            ? rd32(m, 0x00102E12u + (uint32_t)d0 * 4u)
            : rd32(m, 0x00102DAAu + (uint32_t)d0 * 4u);
        sub_0CFA86(m, obj, rec2);

        uint16_t idx44 = rd16(m, obj + 0x44u);
        uint32_t off2 = (d7 < 5)
            ? (0x00102D2Eu + (uint32_t)(int32_t)(int16_t)idx44 * 4u)
            : (0x00102CFEu + (uint32_t)(int32_t)(int16_t)idx44 * 4u);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, off2)));
        uint16_t dy2 = (uint16_t)(rd16(m, off2 + 2u) + rd16(m, obj + 0x68u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + dy2));
        Obj_DrawSprite(m, obj);

        if (rd8(m, obj + 0x23u) == 0u && d7 < 5) {
            wr32(m, obj + 0x68u, 0x00102F4Eu);
            sub_0D09B8(m, obj, 0x00102C76u);
            if (rd8(m, obj + 0x23u) != 0u) {
                uint32_t child = Obj_Spawn(m, obj, 0x00102F52u, 9u, 2u, 0u);
                if ((int32_t)child >= 0) {
                    wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
                    wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
                    uint16_t idx46 = rd16(m, obj + 0x46u);
                    wr16(m, child + 0x10u,
                         rd16(m, 0x00102D5Eu + (uint32_t)(int32_t)(int16_t)idx46 * 2u));
                }
            }
        }
    }

    Obj_DrawSprite(m, rd32(m, obj + 0x08u));
    return 0;
}
