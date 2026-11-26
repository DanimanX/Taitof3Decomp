/* ROM 0x13BDBC: homing missile/dart enemy tick (A0 = object).
 * Steers +0x20 toward target (+0x0C) limited by +0x22 (+0x33/tick), locks
 * heading +0x3C once +0x06 <= 0, picks 16-dir x 2-frame sprite from 0x13BF5E,
 * handles bomb explosion via 0x0CBD12 when on-screen, sets speed +0x42 from
 * +0x06, moves along +0x3C >> 8, queues collision (0x0CBDA4), flashes
 * (0x11C32C), draws (0x001282), checks bounds [-32,320]x[-32,232], decrements
 * +0x06 and re-arms 0x13BDBC. On death or bit4(A5-0x144B), drops into 0x0CD3D6
 * phase 2. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);

    if (spr_hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = setw(r->d[1], d1w);
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x0CBDA4: append obj to both A5+0x8FE and A5+0x906. */
static uint32_t sub_0CBDA4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    a1 += 4u;
    wr32(m, F3_A5 + 0x0906u, a1);
    return a1;
}

int Fn_13BDBC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)         /* 0x13BDBC */
        goto phase2_drop;

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {            /* 0x13BDC6 */
        uint32_t a1 = Obj_0CBDDE(m, obj);                /* 0x13BECE */
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
        goto phase2_drop;
    }

    /* 0x13BDCE..0x13BE08: steer +0x20 toward +0x0C */
    {
        uint32_t tgt = rd32(m, obj + 0x0Cu);
        uint16_t ang = Obj_0CC3C6(m, obj, tgt);          /* 0x13BDD2 */
        int16_t diff = (int16_t)((uint16_t)(ang << 8) - rd16(m, obj + 0x20u));
        int16_t lim = (int16_t)rd16(m, obj + 0x22u);
        if (diff > lim) {
            diff = lim;
        } else {
            int16_t nlim = (int16_t)(-lim);
            if (diff < nlim)
                diff = nlim;
        }
        wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + (uint16_t)diff));
        wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) + 0x0033u));
        if ((int16_t)rd16(m, obj + 0x06u) <= 0)          /* 0x13BDFC */
            wr16(m, obj + 0x3Cu, rd16(m, obj + 0x20u));
    }

    /* 0x13BE0A..0x13BE40: sprite and flip from +0x20 and +0x3A */
    {
        uint16_t d0w = (uint16_t)(rd16(m, obj + 0x20u) + 0x0800u);
        uint16_t fidx = (d0w >> 14) & 3u;                /* bfextu D0{16:2},D1 */
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, 0x000CC10Eu + fidx) ^ 0x03u));
        uint16_t sidx = (uint16_t)(((d0w >> 11) & 0x0Eu)
                                   | ((rd16(m, obj + 0x3Au) >> 1) & 1u));
        wr32(m, obj + 0x10u, rd32(m, 0x0013BF5Eu + (uint32_t)sidx * 4u));
        wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 1u));
    }

    /* 0x13BE44..0x13BE62: bomb check -> 0x0CBD12 */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u
        && (int16_t)rd16(m, obj + 0x14u) >= -0x000A
        && (int16_t)rd16(m, obj + 0x14u) <= 0x011C) {
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
        Obj_ExplodeTick(m, obj);
        r->d[0] = (uint32_t)rd16(m, obj + 0x4Cu);
        r->a[1] = rd32(m, F3_A5 - 0x0CA0u);
        return 0;
    }

    /* 0x13BE68..0x13BE86: speed from +0x06 */
    {
        int16_t t6 = (int16_t)rd16(m, obj + 0x06u);
        if (t6 >= 0) {
            wr16(m, obj + 0x42u, (uint16_t)t6 >> 2);
        } else {
            wr16(m, obj + 0x42u, (uint16_t)(-t6) >> 1);
            wr16(m, obj + 0x22u, 0x0080u);
        }
    }

    /* 0x13BE88..0x13BEA0 */
    Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, obj + 0x3Cu) >> 8));
    (void)sub_0CBDA4(m, obj);
    Obj_11C32C(m, obj);
    draw_sprite_regs(m, r);

    /* 0x13BEA6..0x13BECA: bounds check via cmp2.w at 0x000CC112 / 0x000CC116 */
    {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        r->d[0] = setw(r->d[0], (uint16_t)x);
        if (x < (int16_t)rd16(m, 0x000CC112u) || x > (int16_t)rd16(m, 0x000CC114u)) {
            Obj_Free(m, obj);
            return 0;
        }
        int16_t y = (int16_t)rd16(m, obj + 0x18u);
        r->d[0] = setw(r->d[0], (uint16_t)y);
        if (y < (int16_t)rd16(m, 0x000CC116u) || y > (int16_t)rd16(m, 0x000CC118u)) {
            Obj_Free(m, obj);
            return 0;
        }
    }

    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
    wr32(m, obj, 0x0013BDBCu);                           /* 0x13BDB4 */
    return 0;

phase2_drop:
    /* 0x13BEDE..0x13BEEA -> 0x0CD3D6 with D0 = 2 */
    wr8(m, obj + 0x21u, rd8(m, obj + 0x20u));
    wr8(m, obj + 0x20u, 0u);
    {
        uint32_t a1 = rd32(m, 0x000CD444u + 8u);         /* 0x000CD4D0 */
        uint16_t snd = rd16(m, a1);
        a1 += 2u;
        if (snd != 0u)
            Sound_PlayHooked(m, snd);
        wr32(m, obj + 0x0Cu, a1);
        wr16(m, obj + 0x3Au, 0u);
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x1Eu, 0u);
        wr16(m, obj + 0x1Cu, 0u);
        wr32(m, obj, 0x000CD412u);
        r->d[0] = snd;
        r->a[1] = a1;
    }
    return 0;
}
