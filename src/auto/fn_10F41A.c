/* ROM 0x10F41A: falling/ballistic enemy or debris tick (A0 = object).
 * Handles bomb-clear (0x10F518 -> +0x64 score, 0x0CEF0E), death/pause
 * (0x10F4B2/0x10F4BA -> +0x64 score on hit, sound 0x3B, draw, spawn 0x10F706,
 * 0x0CEE74). Otherwise integrates velocity (+0x50, +0x54) minus scroll into
 * (+0x14, +0x18), dies if Y > 0xE8, tests terrain at (+0x0F, +0x18) -> impact
 * at 0x10F4BA, else steps script/hazard/flash/draw and adds gravity +0x36B0. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
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

int Fn_10F41A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* 0x10F41A: bomb-clear check */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));       /* 0x10F432 */
            Score_AddCurrent(m, 0x64u);                     /* 0x10F518..0x10F51A */
            r->d[0] = 0x00000064u;
            r->a[1] = rd32(m, F3_A5 + 0x1D50u);
            Obj_0CEF0E(m, obj);                             /* 0x10F520 */
            return 0;
        }
    }

    /* 0x10F43E..0x10F44C */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        Score_OnHit(m, obj, 0x64u);                         /* 0x10F4B2..0x10F4B4 */
        goto explode;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        goto explode;

    /* 0x10F450..0x10F46E */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (rd32(m, obj + 0x50u) - rd32(m, F3_A5 + 0x4224u)));
    uint32_t dy = rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u);
    r->d[0] = dy;
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + dy);

    if ((int16_t)rd16(m, obj + 0x18u) > 0x00E8) {           /* 0x10F468..0x10F46E */
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x10F474..10F48C: terrain hit test */
    {
        int16_t tx = (int16_t)(rd16(m, obj + 0x14u) + 0x000F);
        int16_t ty = (int16_t)(rd16(m, obj + 0x18u) + 0x0018);
        r->d[0] = setw(r->d[0], (uint16_t)tx);
        r->d[1] = setw(r->d[1], (uint16_t)ty);
        if (Terrain_TestPoint(m, tx, ty) != 0)
            goto explode;
    }

    /* 0x10F490..10F4B0 */
    Obj_StepScript(m, obj);
    Obj_0D0DC2(m, obj);
    Obj_HitFlash(m, obj);
    draw_sprite_regs(m, r);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + 0x000036B0u);
    return 0;

explode:
    Sound_PlayHooked(m, 0x003Bu);                           /* 0x10F4BA..0x10F4CA */
    draw_sprite_regs(m, r);                                 /* 0x10F4CE */
    {
        uint32_t child = Obj_Spawn(m, obj, 0x0010F706u, 9u, 2u, 0u); /* 0x10F4D4..0x10F4EC */
        r->d[0] = child;
        if ((int32_t)child >= 0) {
            r->a[1] = child;                                /* 0x10F4F8 */
            wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x000Fu));
            wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x000Fu));
        }
    }
    Obj_StartDying(m, obj);                                 /* 0x10F512 */
    return 0;
}
