#include "obj_E1BAE.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
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

/* ROM 0x0D0C8C */
static uint16_t sub_0D0C8C(const f3mem_t *m)
{
    uint16_t d0 = (uint16_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    return rd16(m, 0x0D0D02u + (uint32_t)d0 * 2u);
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

/* ROM 0x0D07B4 */
static void sub_0D07B4(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint32_t muzzle = rd32(m, obj + 0x68u);
    int16_t mx = (int16_t)(uint16_t)(rd16(m, obj + 0x14u) + rd16(m, muzzle));
    if (mx < 0x28 || mx > 0x0127)
        return;
    int16_t my = (int16_t)(uint16_t)(rd16(m, obj + 0x18u) + rd16(m, muzzle + 2u));
    if (my < 0x10 || my > 0xDF)
        return;

    uint32_t players[2] = { 0x41B190u, 0x41B21Au };
    uint16_t adx[2] = { 0u, 0u };
    uint16_t ady[2] = { 0u, 0u };
    uint16_t angs[2] = { 0xFFFFu, 0xFFFFu };
    uint16_t near = rd16(m, a1 + 0x08u);
    uint16_t reach = rd16(m, a1 + 0x0Au);
    uint16_t lo = rd16(m, a1 + 0x0Cu);
    uint16_t hi = rd16(m, a1 + 0x0Eu);

    for (int p = 0; p < 2; p++) {
        uint32_t pl = players[p];
        if ((int16_t)rd16(m, pl + 0x4Cu) < 0)
            continue;

        uint32_t a4 = rd32(m, obj + 0x68u) + 2u;
        uint16_t dy = (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, pl + 0x18u) + rd16(m, a4) - 0x000Fu);
        uint16_t a3w = dy;
        if ((int16_t)dy < 0)
            dy = (uint16_t)(0u - dy);
        ady[p] = dy;
        if (dy > reach)
            continue;

        a4 -= 2u;
        uint16_t dx = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, pl + 0x14u) + rd16(m, a4) - 0x0017u);
        uint16_t a4w = dx;
        if ((int16_t)dx < 0) {
            dx = (uint16_t)(0u - dx);
            adx[p] = dx;
            if (dx > reach)
                continue;
            if (dx < near && dy < near)
                continue;
        } else {
            adx[p] = dx;
            if (dx > reach)
                continue;
            uint16_t lim = (uint16_t)(near - 0x001Eu);
            if ((int16_t)dx < (int16_t)lim && dy < near)
                continue;
        }

        uint16_t ang = sub_0D0E52(m, a4w, a3w);
        if (lo > hi) {
            if (!(ang >= lo || ang <= hi))
                continue;
        } else {
            if (ang < lo || ang > hi)
                continue;
        }
        angs[p] = ang;
    }

    uint16_t d4 = angs[0];
    uint16_t d7 = angs[1];
    uint16_t d0 = d7;

    if ((int16_t)d4 < 0) {
        if ((int16_t)d7 < 0)
            return;
    } else if ((int16_t)d7 < 0) {
        d0 = d4;
    } else {
        uint16_t d2 = (ady[0] > adx[0]) ? ady[0] : adx[0];
        uint16_t d5 = (ady[1] > adx[1]) ? ady[1] : adx[1];
        if (d5 >= d2)
            d0 = d4;
    }

    wr16(m, obj + 0x42u, d0);
    uint16_t d1 = (uint16_t)(rd16(m, a1 + 0x10u) + sub_0D0C8C(m));
    uint16_t roll = Obj_Next4Way(m);
    if ((int16_t)d1 > (int16_t)roll)
        wr8(m, obj + 0x23u, (uint8_t)rd16(m, a1 + 0x12u));
}

/* ROM 0x0DCF04 */

/* ROM 0x0CEF0E: leave the screen (or detonate during a bomb) */
static void sub_0CEF0E(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u) {
        Obj_StartDying(m, obj);
        return;
    }
    if (rd8(m, F3_A5 + 0x1D58u) & 0x02u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_ExplodeTick(m, obj);
            return;
        }
    }
    wr32(m, obj + 0x00u, 0x000CEF44u);
}

/* ROM 0x0CF1D8: start object script */

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

/* ROM 0x0E1BAE: hatch enemy (wait for a shot window, open, launch) */
int Obj_E1BAE_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_RelayTimer(m, 0x00000190u);
            wr8(m, obj + 0x06u, 0u);
            sub_0CEF0E(m, obj);
            return 0;
        }
    }
    {
        int dying = 0;
        if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
            sub_0D0E04(m, obj, 0x00000190u);
            dying = 1;
        } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u || rd8(m, obj + 0x06u) == 5u) {
            dying = 1;
        }
        if (dying) {
            uint32_t d0 = Obj_Spawn(m, obj, 0x000DC43Eu, 3u, 2u, 0u);
            if ((int32_t)d0 >= 0) {
                wr32(m, d0 + 0x14u, rd32(m, obj + 0x14u));
                wr16(m, d0 + 0x14u, (uint16_t)(rd16(m, d0 + 0x14u) + 0x10u));
                wr32(m, d0 + 0x18u, rd32(m, obj + 0x18u));
                wr16(m, d0 + 0x18u, (uint16_t)(rd16(m, d0 + 0x18u) + 0x10u));
                wr16(m, d0 + 0x58u, 0u);
                wr16(m, d0 + 0x60u, 0u);
            }
            wr8(m, obj + 0x06u, 0u);
            Obj_StartDying(m, obj);
            return 0;
        }
    }
    uint8_t st = rd8(m, obj + 0x06u);
    if (st < 1u || st > 3u)
        return -1;   /* state 0 (null entry) / 4 (0x0E1D82) not translated */

    if (rd8(m, obj + 0x22u) != 0u || st != 4u)
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
    if (rd8(m, obj + 0x22u) == 0u) {
        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
        if ((f7 & 0x02u) != 0u) {
            if (ObjScript_CF1E4(m, obj) < 0)
                return -1;
        }
        if (rd8(m, obj + 0x20u) != 0u) {
            if (ObjScript_CF4B8(m, obj, 0x000E1DF6u) < 0)
                return -1;
        }
    }
    sub_0CF7FE(m, obj);
    Obj_FlagOffscreen(m, obj);
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        wr8(m, obj + 0x06u, 0u);
        Obj_StartDying(m, obj);
        return 0;
    }
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    switch (rd8(m, obj + 0x06u)) {
    case 1u:   /* ROM 0x0E1D00 */
        if (rd8(m, obj + 0x23u) == 0u) {
            sub_0D07B4(m, obj, 0x000E1DF6u);
            if (rd8(m, obj + 0x23u) == 0u)
                break;
        }
        {
            uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
            wr8(m, obj + 0x23u, t);
            if (t != 0u)
                break;
            Obj_StartScript(m, obj, 0x000E1E52u);
            wr8(m, obj + 0x06u, 2u);
        }
        break;
    case 2u:   /* ROM 0x0E1D34 */
        if (rd8(m, obj + 0x20u) == 0u) {
            Obj_StartScript(m, obj, 0x000E1E7Cu);
            sub_0D0678(m, obj, 0x000E1EB0u);
            wr8(m, obj + 0x06u, 3u);
        }
        break;
    case 3u:   /* ROM 0x0E1D58 */
        sub_0D0690(m, obj);
        if (rd16(m, obj + 0x4Cu) == 0u) {
            Obj_StartScript(m, obj, 0x000E1E8Eu);
            sub_0D0678(m, obj, 0x000E1ED4u);
            wr8(m, obj + 0x06u, 4u);
        }
        break;
    default:
        return -1;
    }
    sub_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);
    return 0;
}
