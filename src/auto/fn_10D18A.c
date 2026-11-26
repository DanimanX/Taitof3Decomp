/* ROM 0x10D18A: multi-arm enemy/boss object tick (A0 = object).
 * Handles bomb-clear (0x10D28C -> +0x3E8 score, 0x0CEF0E), death/pause
 * (0x10D232/0x10D23E -> +0x3E8 score on hit, spawn 0x0DC46E, 0x0CEE74),
 * script step 0xCF1E4 and movement 0xCF4B8 (aim record 0x10D4F6), Y scroll,
 * state dispatch 0x10D2AC (1: arm on screen, 2: offscreen + countdown +0x4E ->
 * 0x10D5D6 anim, 3: X scroll wait +0x68, 4: X scroll + 4-arm diagonal chain
 * draw), offscreen/+0x22==0xF death, and tail script/hazard/flash/draw. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
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

/* ROM 0x10D308..0x10D4F2: draw one diagonal arm chain segment. */
static uint32_t draw_arm_chain(const f3mem_t *m, f3regs_t *r, uint32_t a4,
                               uint32_t cur, uint32_t frame_ptr,
                               int16_t dx0, int16_t dy0,
                               int16_t step_x, int16_t step_y,
                               int cmp_right)
{
    uint32_t arm = rd32(m, cur + 0x54u);
    if (rd8(m, arm + 0x06u) == 0u)
        return arm;

    uint32_t saved_10 = rd32(m, arm + 0x10u);
    uint8_t saved_1c = rd8(m, arm + 0x1Cu);
    uint32_t saved_14 = rd32(m, arm + 0x14u);
    uint32_t saved_18 = rd32(m, arm + 0x18u);

    wr32(m, arm + 0x14u, saved_14 - rd32(m, F3_A5 + 0x4224u));
    wr32(m, arm + 0x18u, saved_18 - rd32(m, F3_A5 + 0x4228u));
    r->a[0] = arm;
    draw_sprite_regs(m, r);

    Obj_SetFrame(m, arm, frame_ptr);
    wr16(m, arm + 0x14u, (uint16_t)((int16_t)rd16(m, arm + 0x14u) + dx0));
    wr16(m, arm + 0x18u, (uint16_t)((int16_t)rd16(m, arm + 0x18u) + dy0));

    for (;;) {
        int16_t lim = (int16_t)rd16(m, a4 + 0x3Au);
        int16_t x = (int16_t)rd16(m, arm + 0x14u);
        if (cmp_right) {
            if (lim < x)
                break;
        } else {
            if ((int16_t)(lim + 0x10) > x)
                break;
        }
        draw_sprite_regs(m, r);
        wr16(m, arm + 0x14u, (uint16_t)((int16_t)rd16(m, arm + 0x14u) + step_x));
        wr16(m, arm + 0x18u, (uint16_t)((int16_t)rd16(m, arm + 0x18u) + step_y));
    }

    wr32(m, arm + 0x18u, saved_18);
    wr32(m, arm + 0x14u, saved_14);
    wr8(m, arm + 0x1Cu, saved_1c);
    wr32(m, arm + 0x10u, saved_10);
    return arm;
}

int Fn_10D18A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* 0x10D18A: bomb-clear check */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));       /* 0x10D1A2 */
            Score_AddCurrent(m, 0x03E8u);                   /* 0x10D28C..0x10D292 */
            wr8(m, obj + 0x06u, 0u);                        /* 0x10D298 */
            r->d[0] = 0x03E8u;
            r->a[1] = rd32(m, F3_A5 + 0x1D50u);
            Obj_0CEF0E(m, obj);                             /* 0x10D29C */
            return 0;
        }
    }

    /* 0x10D1AE..0x10D1BC */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        Score_OnHit(m, obj, 0x03E8u);                       /* 0x10D232..0x10D238 */
        goto spawn_die;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        goto spawn_die;

    /* 0x10D1C0 */
    {
        uint8_t f7 = rd8(m, obj + 0x07u);
        if ((f7 & 0x02u) != 0u) {
            wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
            if (ObjScript_CF1E4(m, obj) < 0)                /* 0x10D1CA */
                return -1;
        }
    }

    /* 0x10D1D0 */
    if (rd8(m, obj + 0x20u) != 0u) {
        if (ObjScript_CF4B8(m, obj, 0x0010D4F6u) < 0)       /* 0x10D1DC */
            return -1;
    }

    /* 0x10D1E2 */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));

    /* 0x10D1EA..0x10D1F0: state dispatch 0x10D2AC */
    switch (rd8(m, obj + 0x06u)) {
    case 1u:
        Obj_ArmOnScreen(m, obj);                            /* 0x10D2C0 */
        break;
    case 2u:
        Obj_FlagOffscreen(m, obj);                          /* 0x10D2C8 */
        {
            uint8_t c = (uint8_t)(rd8(m, obj + 0x4Eu) - 1u);
            wr8(m, obj + 0x4Eu, c);                         /* 0x10D2CE */
            if (c == 0u) {
                Obj_SetAnim(m, obj, 0x0010D5D6u);           /* 0x10D2DA */
                wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u)); /* 0x10D2E0 */
            }
        }
        break;
    case 3u:
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - rd32(m, F3_A5 + 0x4224u)); /* 0x10D2E8 */
        if (rd16(m, obj + 0x68u) != 0u)                     /* 0x10D2F0 */
            wr8(m, obj + 0x06u, 4u);                        /* 0x10D2F8 */
        break;
    case 4u:
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - rd32(m, F3_A5 + 0x4224u)); /* 0x10D300 */
        {
            uint32_t cur = obj;
            cur = draw_arm_chain(m, r, obj, cur, 0x0010D606u, -0x0E,  0x0E, -0x10,  0x10, 0);
            cur = draw_arm_chain(m, r, obj, cur, 0x0010D610u,  0x0E,  0x0E,  0x10,  0x10, 1);
            cur = draw_arm_chain(m, r, obj, cur, 0x0010D61Au,  0x0E, -0x0E,  0x10, -0x10, 1);
            (void)draw_arm_chain(m, r, obj, cur, 0x0010D624u, -0x0E, -0x0E, -0x10, -0x10, 0);
            r->a[0] = obj;
        }
        break;
    default:
        return -1;
    }

    /* 0x10D1F8 */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u)
        goto die_only;

    /* 0x10D202 */
    if (rd8(m, obj + 0x22u) == 0x0Fu) {
        if ((int16_t)rd16(m, obj + 0x3Au) < -0x20)          /* 0x10D2A2..0x10D2A8 */
            goto die_only;
        goto spawn_die;
    }

    /* 0x10D20C..0x10D230 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    Obj_StepScript(m, obj);                                 /* 0x10D218 */
    Obj_0D0DC2(m, obj);                                     /* 0x10D21E */
    Obj_HitFlash(m, obj);                                   /* 0x10D224 */
    draw_sprite_regs(m, r);                                 /* 0x10D22A */
    return 0;

spawn_die:
    {
        uint32_t child = Obj_Spawn(m, obj, 0x000DC46Eu, 3u, 2u, 0u); /* 0x10D23E..0x10D256 */
        r->d[0] = child;
        if ((int32_t)child >= 0) {
            r->a[1] = child;
            wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x0010u));
            wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x0010u));
            wr16(m, child + 0x58u, 0u);
            wr16(m, child + 0x60u, 0u);
        }
    }

die_only:
    wr8(m, obj + 0x06u, 0u);                                /* 0x10D282 */
    Obj_StartDying(m, obj);                                 /* 0x10D286 */
    return 0;
}
