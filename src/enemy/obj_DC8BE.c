#include "obj_DC8BE.h"
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
    if (!(flags & 0x80u))
        return;

    int16_t x = (int16_t)rd16(m, obj + 0x3Au);
    if (x < -0x40 || x > 0x014F)
        return;

    flags = (uint8_t)((flags & (uint8_t)~0x80u) | 0x02u);
    wr8(m, obj + 0x07u, flags);
}

/* ROM 0x0CFA7C */
static void sub_0CFA7C(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d0)
{
    uint16_t off = rd16(m, 0x0CFA96u + (uint32_t)((int32_t)(int16_t)d0 * 2));
    a1 += (uint32_t)(int32_t)(int16_t)off;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D0090 */
static void sub_0D0090(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d0)
{
    uint16_t off = rd16(m, 0x0D00B4u + (uint32_t)((int32_t)(int16_t)d0 * 2));
    a1 += (uint32_t)(int32_t)(int16_t)off;
    a1 = rd32(m, a1);
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

/* ROM 0x0D0C8C */
static uint16_t sub_0D0C8C(const f3mem_t *m)
{
    uint16_t idx = (uint16_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    return rd16(m, 0x0D0D02u + (uint32_t)idx * 2u);
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

    uint16_t src = (d1 <= d0) ? d0 : d1;
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
        unsigned cnt = (unsigned)d3 & 63u;
        d0 = (cnt >= 16u) ? 0u : (uint16_t)(d0 >> cnt);
        d1 = (cnt >= 16u) ? 0u : (uint16_t)(d1 >> cnt);
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
    uint32_t off_ptr = rd32(m, obj + 0x68u);
    int16_t cx = (int16_t)(uint16_t)(rd16(m, obj + 0x14u) + rd16(m, off_ptr));
    if (cx < 0x28 || cx > 0x0127)
        return;
    int16_t cy = (int16_t)(uint16_t)(rd16(m, obj + 0x18u) + rd16(m, off_ptr + 2u));
    if (cy < 0x10 || cy > 0xDF)
        return;

    uint16_t d4 = 0xFFFFu;
    uint16_t d2 = 0u;
    uint16_t d3 = 0u;
    {
        uint32_t a2 = 0x41B190u;
        if ((int16_t)rd16(m, a2 + 0x4Cu) >= 0) {
            uint32_t a4 = rd32(m, obj + 0x68u) + 2u;
            d3 = (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, a2 + 0x18u)
                            + rd16(m, a4) - 0x000Fu);
            uint16_t a3_dy = d3;
            if ((int16_t)d3 < 0)
                d3 = (uint16_t)(0u - d3);
            if (d3 <= rd16(m, a1 + 0x0Au)) {
                a4 -= 2u;
                d2 = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, a2 + 0x14u)
                                + rd16(m, a4) - 0x0017u);
                uint16_t a4_dx = d2;
                int ok = 0;
                if ((int16_t)d2 < 0) {
                    d2 = (uint16_t)(0u - d2);
                    if (d2 <= rd16(m, a1 + 0x0Au) &&
                        (d2 >= rd16(m, a1 + 0x08u) || d3 >= rd16(m, a1 + 0x08u))) {
                        ok = 1;
                    }
                } else {
                    if (d2 <= rd16(m, a1 + 0x0Au)) {
                        int16_t lim = (int16_t)(uint16_t)(rd16(m, a1 + 0x08u) - 0x001Eu);
                        if ((int16_t)d2 >= lim || d3 >= rd16(m, a1 + 0x08u))
                            ok = 1;
                    }
                }
                if (ok) {
                    uint16_t ang = sub_0D0E52(m, a4_dx, a3_dy);
                    uint16_t lo = rd16(m, a1 + 0x0Cu);
                    uint16_t hi = rd16(m, a1 + 0x0Eu);
                    if (lo <= hi) {
                        if (ang >= lo && ang <= hi)
                            d4 = ang;
                    } else {
                        if (ang >= lo || ang <= hi)
                            d4 = ang;
                    }
                }
            }
        }
    }

    uint16_t d7 = 0xFFFFu;
    uint16_t d5 = 0u;
    uint16_t d6 = 0u;
    {
        uint32_t a2 = 0x41B21Au;
        if ((int16_t)rd16(m, a2 + 0x4Cu) >= 0) {
            uint32_t a4 = rd32(m, obj + 0x68u) + 2u;
            d6 = (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, a2 + 0x18u)
                            + rd16(m, a4) - 0x000Fu);
            uint16_t a3_dy = d6;
            if ((int16_t)d6 < 0)
                d6 = (uint16_t)(0u - d6);
            if (d6 <= rd16(m, a1 + 0x0Au)) {
                a4 -= 2u;
                d5 = (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, a2 + 0x14u)
                                + rd16(m, a4) - 0x0017u);
                uint16_t a4_dx = d5;
                int ok = 0;
                if ((int16_t)d5 < 0) {
                    d5 = (uint16_t)(0u - d5);
                    if (d5 <= rd16(m, a1 + 0x0Au) &&
                        (d5 >= rd16(m, a1 + 0x08u) || d6 >= rd16(m, a1 + 0x08u))) {
                        ok = 1;
                    }
                } else {
                    if (d5 <= rd16(m, a1 + 0x0Au)) {
                        int16_t lim = (int16_t)(uint16_t)(rd16(m, a1 + 0x08u) - 0x001Eu);
                        if ((int16_t)d5 >= lim || d6 >= rd16(m, a1 + 0x08u))
                            ok = 1;
                    }
                }
                if (ok) {
                    uint16_t ang = sub_0D0E52(m, a4_dx, a3_dy);
                    uint16_t lo = rd16(m, a1 + 0x0Cu);
                    uint16_t hi = rd16(m, a1 + 0x0Eu);
                    if (lo <= hi) {
                        if (ang >= lo && ang <= hi)
                            d7 = ang;
                    } else {
                        if (ang >= lo || ang <= hi)
                            d7 = ang;
                    }
                }
            }
        }
    }

    uint16_t chosen = d7;
    if ((int16_t)d4 < 0) {
        if ((int16_t)d7 < 0)
            return;
    } else if ((int16_t)d7 < 0) {
        chosen = d4;
    } else {
        if (d3 > d2)
            d2 = d3;
        if (d6 > d5)
            d5 = d6;
        if (d5 >= d2)
            chosen = d4;
    }

    wr16(m, obj + 0x42u, chosen);
    uint16_t d1 = (uint16_t)(rd16(m, a1 + 0x10u) + sub_0D0C8C(m));
    uint16_t rng = Obj_Next4Way(m);
    if ((int16_t)d1 <= (int16_t)rng)
        return;
    wr8(m, obj + 0x23u, (uint8_t)rd16(m, a1 + 0x12u));
}

/* ROM 0x0D0BBC */
static void sub_0D0BBC(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t target = rd16(m, obj + 0x42u);
    uint16_t d0 = (uint16_t)(target - rd16(m, obj + 0x08u));
    if ((int16_t)d0 < 0)
        d0 = (uint16_t)(d0 + 0x0168u);

    if (d0 <= 0x00B4u) {
        if (d0 <= rd16(m, a1 + 0x04u)) {
            wr16(m, obj + 0x08u, target);
            return;
        }
        uint32_t cur = rd32(m, obj + 0x08u) + rd32(m, a1 + 0x04u);
        wr32(m, obj + 0x08u, cur);
        if (rd16(m, obj + 0x08u) >= 0x0168u)
            wr16(m, obj + 0x08u, (uint16_t)(rd16(m, obj + 0x08u) - 0x0168u));
        return;
    }

    d0 = (uint16_t)((uint16_t)(0u - d0) + 0x0168u);
    if (d0 <= rd16(m, a1 + 0x04u)) {
        wr16(m, obj + 0x08u, target);
        return;
    }
    uint32_t cur = rd32(m, obj + 0x08u) - rd32(m, a1 + 0x04u);
    wr32(m, obj + 0x08u, cur);
    if ((int32_t)cur < 0)
        wr16(m, obj + 0x08u, (uint16_t)(rd16(m, obj + 0x08u) + 0x0168u));
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
    uint8_t hit_by = rd8(m, obj + 0x35u);
    if (hit_by == 1u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        a1 = rd32(m, a1 + 0x08u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    } else if (hit_by == 2u) {
        uint32_t a1 = rd32(m, obj + 0x36u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
    }
}

/* ROM 0x0D0E30 */

/* ROM 0x0DC9D2 */
static void sub_0DC9D2(const f3mem_t *m, uint32_t obj)
{
    sub_0CF7FE(m, obj);
    uint8_t flags = rd8(m, obj + 0x07u);
    if (!(flags & 0x80u)) {
        wr8(m, obj + 0x07u, (uint8_t)(flags | 0x10u));
        wr8(m, obj + 0x06u, 0x01u);
    }
}

/* ROM 0x0DC9F0 */
static void sub_0DC9F0(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x23u) == 0u) {
        sub_0D07B4(m, obj, 0x0DCAEEu);
        sub_0D0BBC(m, obj, 0x0DCAEEu);
        sub_0CFA7C(m, obj, 0x0DCB26u, rd16(m, obj + 0x08u));
        if (rd8(m, obj + 0x23u) == 0u)
            return;
    }

    uint8_t cnt = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, cnt);
    if (cnt == 0u) {
        sub_0D0090(m, obj, 0x0DCBC6u, rd16(m, obj + 0x08u));
        wr8(m, obj + 0x06u, 0x02u);
    }
}

/* ROM 0x0DCA3C */
static void sub_0DCA3C(const f3mem_t *m, uint32_t obj)
{
    sub_0D0690(m, obj);
    if (rd16(m, obj + 0x4Cu) == 0u) {
        sub_0CFA7C(m, obj, 0x0DCB26u, rd16(m, obj + 0x08u));
        wr8(m, obj + 0x23u, 0x0Au);
        wr8(m, obj + 0x06u, 0x03u);
        return;
    }

    uint16_t d1 = rd16(m, obj + 0x68u);
    if ((int16_t)d1 < 0)
        return;

    uint32_t a1 = Obj_Spawn(m, obj, 0x000D9224u, 9u, 2u, 0u);
    if ((int32_t)a1 < 0)
        return;

    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x001Du));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0024u));
    wr16(m, a1 + 0x58u, d1);
    wr16(m, a1 + 0x10u, 0x0010u);
}

/* ROM 0x0DCAB6 */
static void sub_0DCAB6(const f3mem_t *m, uint32_t obj)
{
    uint8_t cnt = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, cnt);
    if (cnt == 0u) {
        wr8(m, obj + 0x06u, 0x01u);
        return;
    }

    uint8_t saved = rd8(m, obj + 0x23u);
    sub_0D07B4(m, obj, 0x0DCAEEu);
    wr8(m, obj + 0x23u, saved);
    sub_0D0BBC(m, obj, 0x0DCAEEu);
    sub_0CFA7C(m, obj, 0x0DCB26u, rd16(m, obj + 0x08u));
}

/* ROM 0x0DC8BE */
int Obj_DC8BE_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x1D58u) & 0x02u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_RelayTimer(m, 0x00000320u);
            wr32(m, obj + 0x10u, 0x00063F80u);
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0014u));
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0014u));
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        sub_0D0E04(m, obj, 0x00000320u);
        goto death_spawn;
    }
    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u)
        goto death_spawn;

    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));

    Obj_FlagOffscreen(m, obj);
    if (rd8(m, obj + 0x07u) & 0x08u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    switch (rd8(m, obj + 0x06u)) {
    case 0u:
        sub_0DC9D2(m, obj);
        break;
    case 1u:
        sub_0DC9F0(m, obj);
        break;
    case 2u:
        sub_0DCA3C(m, obj);
        break;
    case 3u:
        sub_0DCAB6(m, obj);
        break;
    default:
        /* unimplemented: ROM 0x0DC928 */
        return -1;
    }

    sub_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);
    return 0;

death_spawn:
    {
        uint32_t a1 = Obj_Spawn(m, obj, 0x000DC456u, 3u, 2u, 0u);
        if ((int32_t)a1 >= 0) {
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0024u));
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0026u));
            wr16(m, a1 + 0x58u, 0u);
            wr16(m, a1 + 0x60u, 0u);
        }
        Obj_StartDying(m, obj);
        return 0;
    }
}
