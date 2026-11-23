#include "obj_EB2EA.h"
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

/* ROM 0x0D0050 */
static void sub_0D0050(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    wr32(m, a1 + 0x28u, 0u);
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
    int16_t d0 = (int16_t)rd16(m, a1);
    a1 += 2u;
    if (d0 >= 0) {
        wr16(m, obj + 0x4Cu, (uint16_t)d0);
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
        a1 += 2u;
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        return;
    }

    uint16_t rewind = rd16(m, a1);
    if (rewind != 0u) {
        a1 -= rewind;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
        a1 += 2u;
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
    }
}

/* ROM 0x0D0C9E */
static uint16_t sub_0D0C9E(const f3mem_t *m)
{
    uint16_t d0 = (uint16_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    return rd16(m, 0x000D0D42u + (uint32_t)d0 * 2u);
}

/* ROM 0x0D0DE4 */
static void sub_0D0DE4(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, obj + 0x07u) & 0x80u) != 0u)
        return;
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x0906u, a1 + 4u);
}

/* ROM 0x0D0E04 */
static void sub_0D0E04(const f3mem_t *m, uint32_t obj, uint32_t d0)
{
    uint8_t s = rd8(m, obj + 0x35u);
    if (s == 1u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        a1 = rd32(m, a1 + 0x08u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    } else if (s == 2u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    }
}

/* ROM 0x0D0E30 */

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
    uint16_t roll = Obj_Next4Way(m);
    if ((int16_t)d1 > (int16_t)roll)
        wr8(m, obj + 0x23u, (uint8_t)rd16(m, a1 + 0x12u));
}

/* ROM 0x0EB2EA */
int Obj_EB2EA_Tick(const f3mem_t *m, uint32_t obj)
{
    int dying = 0;
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        sub_0D0E04(m, obj, 0x000000C8u);
        dying = 1;
    } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        dying = 1;
    }

    if (dying) {
        uint32_t d0 = Obj_Spawn(m, obj, 0x000DC586u, 3u, 2u, 0u);
        if ((int32_t)d0 >= 0) {
            uint32_t a1 = d0;
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 8u));
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 8u));
            wr16(m, a1 + 0x58u, rd16(m, obj + 0x58u));
            wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));
        }

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

    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));

    uint8_t f7 = rd8(m, obj + 0x07u);
    wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
    if ((f7 & 0x02u) != 0u) {
        if (ObjScript_CF1E4(m, obj) < 0)
            return -1;
    }

    if (rd8(m, obj + 0x20u) != 0u) {
        if (ObjScript_CF4B8(m, obj, 0x000EB578u) < 0)
            return -1;
    }

    sub_0CF7FE(m, obj);
    Obj_FlagOffscreen(m, obj);
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr8(m, a1 + 0x22u, (uint8_t)(rd8(m, a1 + 0x22u) - 1u));
        wr8(m, a1 + 0x07u, (uint8_t)(rd8(m, a1 + 0x07u) | 0x01u));
        Obj_StartDying(m, obj);
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    switch (rd8(m, obj + 0x06u)) {
    case 0u:
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {
            sub_0D0050(m, obj);
            sub_0D005A(m, obj, 0x000EB5F8u, rd16(m, obj + 0x58u));
            wr8(m, obj + 0x06u, 0x01u);
        }
        break;
    case 1u:
        if (rd8(m, obj + 0x23u) == 0u) {
            if ((rd8(m, obj + 0x07u) & 0x20u) == 0u)
                break;
            sub_0D09B8(m, obj, 0x000EB578u);
            if (rd8(m, obj + 0x23u) == 0u)
                break;
        }
        {
            uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
            wr8(m, obj + 0x23u, t);
            if (t != 0u)
                break;
            int32_t d0 = (int32_t)Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
            if (d0 < 0)
                break;
            uint32_t a1 = (uint32_t)d0;
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0010u));
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0010u));
            wr16(m, a1 + 0x58u, rd16(m, obj + 0x42u));
            wr16(m, a1 + 0x10u, 0u);
            wr8(m, obj + 0x23u, 0x1Eu);
            wr8(m, obj + 0x06u, 0x02u);
        }
        break;
    case 2u:
        {
            uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
            wr8(m, obj + 0x23u, t);
            if (t == 0u)
                wr8(m, obj + 0x06u, 0x01u);
        }
        break;
    default:
        return -1;
    }

    sub_0D005A(m, obj, 0x000EB5F8u, rd16(m, obj + 0x58u));
    sub_0D0690(m, obj);
    sub_0D0DE4(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_RelayTimer(m, 0x000000C8u);
            uint32_t a1 = rd32(m, obj + 0x08u);
            uint8_t rem = (uint8_t)(rd8(m, a1 + 0x22u) - 1u);
            wr8(m, a1 + 0x22u, rem);
            if (rem != 0u || (rd8(m, a1 + 0x07u) & 0x01u) != 0u) {
                sub_0CEE9A(m, obj, 0x000000C8u);
                return 0;
            }
            if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
                Obj_StartDying(m, obj);
                return 0;
            }
            Obj_RelayTimer(m, 0x000007D0u);
            uint32_t d0 = Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x000007D0u);
            sub_0CEEA2(m, obj, d0);
            return 0;
        }
    }

    return 0;
}
