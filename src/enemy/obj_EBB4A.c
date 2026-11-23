#include "obj_EBB4A.h"
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

/* ROM 0x0D25E4 */
static int sub_0D25E4(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    uint16_t d4w = 0u;

    while (d1w >= 0x0032u) {
        d4w = (uint16_t)(d4w + 1u);
        d1w = (uint16_t)(d1w - 0x0032u);
    }

    uint16_t d2w = d1w;
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    uint32_t d1 = rd32(m, 0x000D2044u + off);
    uint32_t d0 = rd32(m, 0x000D1EDCu + off);

    /* Jump table 0x0D4F26 [0..50] -> 0x0D4FF2..0x0D538A: exact (D0*k, D1*k) mod 2^32. */
    d0 = (uint32_t)(d0 * (uint32_t)d2w);
    d1 = (uint32_t)(d1 * (uint32_t)d2w);

    uint32_t d3 = d0;
    d2w = d4w;
    uint32_t d4 = d1;

    d1 = rd32(m, 0x000D27A4u + off);
    d0 = rd32(m, 0x000D263Cu + off);

        /* Jump table 0x0D4F26 [0..50] -> 0x0D4FF2..0x0D538A: block k is the
         * constant 0..50 (k=0 at 0x0D4FF2 clears both), so (D0*k, D1*k)
         * mod 2^32 is exact for every k the table has. */

    d0 = (uint32_t)(d0 * (uint32_t)d2w);
    d1 = (uint32_t)(d1 * (uint32_t)d2w);

    *out_d0 = d0 + d3;
    *out_d1 = d1 + d4;
    return 0;
}

/* ROM 0x0D0E3A */

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

    if (d2 != 2u && d2 != 4u) {
        uint16_t tmp = d0;
        d0 = d1;
        d1 = tmp;
    }

    d1 = (uint16_t)((uint16_t)(d1 << 6) + d0);
    d0 = (uint16_t)rd8(m, 0x0D0EBCu + (uint32_t)(int32_t)(int16_t)d1);
    d0 = (uint16_t)(d0 + rd16(m, 0x0D0EB4u + (uint32_t)(int32_t)(int16_t)d2));
    if (d0 == 0x0168u)
        d0 = 0u;
    return d0;
}

/* ROM 0x0D0C9E */
static uint16_t sub_0D0C9E(const f3mem_t *m)
{
    uint16_t d0 = (uint16_t)(rd8(m, F3_A5 + 0x87Fu) >> 3);
    return rd16(m, 0x000D0D42u + (uint32_t)d0 * 2u);
}

/* ROM 0x0D0678: script pointer + first frame */
static void sub_0D0678(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x48u, a1);
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
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, cnt);
    if (cnt != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0 < 0) {
        int16_t back = (int16_t)rd16(m, a1);
        if (back == 0)
            return;
        a1 -= (uint32_t)(int32_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
        a1 += 2u;
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        return;
    }

    wr16(m, obj + 0x4Cu, d0);
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D0DC2 */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    Coll_AddHazard(m, obj);
}

/* ROM 0x0D0E04 */
static void sub_0D0E04(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t tag = rd8(m, obj + 0x35u);
    if (tag == 1u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        a1 = rd32(m, a1 + 0x08u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    } else if (tag == 2u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    }
}

/* ROM 0x0D0E30 */

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

/* ROM 0x0CF82C */
static void sub_0CF82C(const f3mem_t *m, uint32_t obj)
{
    uint8_t f = rd8(m, obj + 0x07u);
    if ((f & 0x80u) == 0u)
        return;
    int16_t y = (int16_t)rd16(m, obj + 0x3Eu);
    if (y < -0x40 || y > 0xF7)
        return;
    wr8(m, obj + 0x07u, (uint8_t)((f & ~0x80u) | 0x02u));
}

static void wrap360(const f3mem_t *m, uint32_t a, uint32_t step)
{
    uint32_t v = rd32(m, a) + rd32(m, step);
    wr32(m, a, v);
    if ((int32_t)v < 0)
        wr16(m, a, (uint16_t)(rd16(m, a) + 0x168u));
    else if (rd16(m, a) >= 0x168u)
        wr16(m, a, (uint16_t)(rd16(m, a) - 0x168u));
}

/* ROM 0x0CF98A: orbit offset (D0, D1) from the motion block at +0xC */
static int sub_0CF98A(const f3mem_t *m, uint32_t obj, uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    const uint16_t d6w = 0x168u;
    wrap360(m, a1 + 0x08u, a1 + 0x0Cu);
    uint32_t v = rd32(m, a1 + 0x10u) + rd32(m, a1 + 0x14u);
    wr32(m, a1 + 0x10u, v);
    if ((int32_t)v < 0)
        wr32(m, a1 + 0x10u, 0u);
    else if (rd16(m, a1 + 0x10u) > 0x0FFFu)
        wr16(m, a1 + 0x10u, 0x0FFFu);
    wrap360(m, a1 + 0x18u, a1 + 0x1Cu);
    wrap360(m, a1 + 0x20u, a1 + 0x24u);

    uint16_t d0w = rd16(m, a1 + 0x08u), d1w = rd16(m, a1 + 0x10u);
    uint16_t d2w = d0w, d3w = d1w;
    uint32_t d0, d1;
    if (sub_0D25E4(m, d0w, d1w, &d0, &d1) < 0)
        return -1;
    uint32_t d4 = d0, d5 = d1;
    d2w = (uint16_t)(d2w - rd16(m, a1 + 0x18u));
    if ((int16_t)d2w < 0)
        d2w = (uint16_t)(d2w + d6w);
    if (sub_0D25E4(m, d2w, d3w, &d0, &d1) < 0)
        return -1;
    d0w = rd16(m, a1 + 0x20u);
    d1w = (uint16_t)(d1 >> 16);
    d3w = 0x5Au;
    if (d2w < 0xB4u) {
        d1w = (uint16_t)(0u - d1w);
        d3w = (uint16_t)(d3w + 0xB4u);
    }
    d2w = d1w;
    if (sub_0D25E4(m, d0w, d1w, &d0, &d1) < 0)
        return -1;
    d0w = (uint16_t)(rd16(m, a1 + 0x18u) + d3w);
    if (d0w >= d6w)
        d0w = (uint16_t)(d0w - d6w);
    d1w = (uint16_t)((uint16_t)(d1 >> 16) + d2w);
    if (sub_0D25E4(m, d0w, d1w, &d0, &d1) < 0)
        return -1;
    *out_d0 = d0 + d4;
    *out_d1 = d1 + d5;
    return 0;
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

/* ROM 0x0EBB4A: orbiting formation member that aims shots */
int Obj_EBB4A_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0;
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_RelayTimer(m, 0xC8u);
            uint32_t p = rd32(m, obj + 0x08u);
            uint8_t n = (uint8_t)(rd8(m, p + 0x22u) - 1u);
            wr8(m, p + 0x22u, n);
            if (n != 0u || (rd8(m, p + 0x07u) & 0x01u) != 0u) {
                sub_0CEE9A(m, obj, 0xC8u);
                return 0;
            }
            if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
                Obj_StartDying(m, obj);
                return 0;
            }
            Obj_RelayTimer(m, 0x7D0u);
            d0 = Obj_Spawn(m, obj, 0x117B44u, 0u, 1u, 0x7D0u);
            sub_0CEEA2(m, obj, d0);
            return 0;
        }
    }
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        sub_0D0E04(m, obj, 0xC8u);
        goto burst;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        goto burst;
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
    {
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
        if ((f7 & 0x02u) != 0u && ObjScript_CF1E4(m, obj) < 0)
            return -1;
    }
    if (rd8(m, obj + 0x20u) != 0u && ObjScript_CF4B8(m, obj, 0xEBD98u) < 0)
        return -1;
    sub_0CF82C(m, obj);
    Obj_FlagOffscreen(m, obj);
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        uint32_t p = rd32(m, obj + 0x08u);
        wr8(m, p + 0x22u, (uint8_t)(rd8(m, p + 0x22u) - 1u));
        wr8(m, p + 0x07u, (uint8_t)(rd8(m, p + 0x07u) | 0x01u));
        Obj_StartDying(m, obj);
        return 0;
    }
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    {
        uint32_t o0, o1;
        if (sub_0CF98A(m, obj, &o0, &o1) < 0)
            return -1;
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + o1);
    }
    {
        uint16_t ang = rd16(m, rd32(m, obj + 0x0Cu) + 0x08u);
        if (ang == 0x5Au)
            sub_0D0678(m, obj, 0xEBDFAu);
        else if (ang == 0x10Eu)
            sub_0D0678(m, obj, 0xEBE36u);
    }
    sub_0D0690(m, obj);
    sub_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);
    {
        uint8_t t = rd8(m, obj + 0x23u);
        if (t != 0u) {
            t = (uint8_t)(t - 1u);
            wr8(m, obj + 0x23u, t);
            if (t != 0u)
                return 0;
        }
        sub_0D09B8(m, obj, 0xEBD98u);
        if (rd8(m, obj + 0x23u) == 0u)
            return 0;
        uint32_t c = Obj_Spawn(m, obj, 0xD9092u, 9u, 2u, 0u);
        if ((int32_t)c < 0)
            return 0;
        wr32(m, c + 0x14u, rd32(m, obj + 0x14u));
        wr16(m, c + 0x14u, (uint16_t)(rd16(m, c + 0x14u) + 8u));
        wr32(m, c + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, c + 0x18u, (uint16_t)(rd16(m, c + 0x18u) + 8u));
        wr16(m, c + 0x58u, rd16(m, obj + 0x42u));
        wr16(m, c + 0x10u, 0u);
        return 0;
    }
burst:
    d0 = Obj_Spawn(m, obj, 0xDC586u, 3u, 2u, 0u);
    if ((int32_t)d0 >= 0) {
        wr32(m, d0 + 0x14u, rd32(m, obj + 0x14u));
        wr16(m, d0 + 0x14u, (uint16_t)(rd16(m, d0 + 0x14u) + 8u));
        wr32(m, d0 + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, d0 + 0x18u, (uint16_t)(rd16(m, d0 + 0x18u) + 8u));
        wr16(m, d0 + 0x58u, rd16(m, obj + 0x58u));
        wr16(m, d0 + 0x60u, rd16(m, obj + 0x60u));
    }
    {
        uint32_t p = rd32(m, obj + 0x08u);
        uint8_t n = (uint8_t)(rd8(m, p + 0x22u) - 1u);
        wr8(m, p + 0x22u, n);
        if (n != 0u || (rd8(m, p + 0x07u) & 0x01u) != 0u) {
            sub_0CEE48(m, obj, d0);
            return 0;
        }
        if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
            Obj_StartDying(m, obj);
            return 0;
        }
        sub_0D0E04(m, obj, 0x7D0u);
        d0 = Obj_Spawn(m, obj, 0x117B44u, 0u, 1u, 0x7D0u);
        sub_0CEE52(m, obj, d0);
        return 0;
    }
}
