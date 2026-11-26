/* ROM 0x13C6DE: multi-part boss/enemy wing-walker tick (A0 = object).
 * Integrates (+0x50, +0x52) velocity (0x13CA14), accelerates +0x52 by +0x42,
 * and steps through three phases:
 * - 0x13C6DE: animates 0x13CA40, draws main + (+0x50,0) wing + (+0x17,-10/+26)
 *   thruster sprites, queues collision (0x0CBDA4), re-arms 0x13C6DE; on
 *   0x13CA40 end (-1), clears +0x3A/+0x3E and falls into 0x13C7C0.
 * - 0x13C7C0: animates 0x13CA54, draws 0x3C5B0 + (+0x50,0) sprite, queues
 *   collision, re-arms 0x13C852; on end falls into 0x13C85E.
 * - 0x13C85E: cycles 0x13CA74 sprites on +0x3A/+0x3E, optionally fires three
 *   0x0CCD04 shots (0x0CC2BC), decelerates +0x50 by 0x20, queues collision,
 *   frees at X <= -0x32, else re-arms 0x13C9AA.
 * Bomb check routes to 0x13CA06 + 0x0CBD12; death or bit4(A5-0x144B) spawns
 * 0x13E4EE + sound 0x16 and installs 0x13C9F8. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
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

/* ROM 0x0CC2BC: D0 = (rem << 16) | quot, D1 = swapped scaled period. */
static void sub_0CC2BC(const f3mem_t *m, uint16_t d0w, uint32_t *d0_out, uint32_t *d1_out)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = ((uint32_t)d0w * (uint32_t)d1w) << 8;
    d1 = (d1 << 16) | (d1 >> 16);
    if ((uint16_t)d1 == 0u)
        d1 = (d1 & 0xFFFF0000u) | 1u;
    uint32_t d0 = (uint32_t)rd16(m, F3_A5 + 0x0878u);
    uint16_t div = (uint16_t)d1;
    uint16_t quot = (uint16_t)(d0 / div);
    uint16_t rem = (uint16_t)(d0 % div);
    *d0_out = ((uint32_t)quot << 16) | rem;
    *d1_out = d1;
}

/* ROM 0x13CA14: integrate (+0x50, +0x52) << 6 into (+0x14, +0x18). */
static void sub_13CA14(const f3mem_t *m, uint32_t obj)
{
    int32_t dx = (int32_t)(int16_t)rd16(m, obj + 0x50u) << 6;
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (uint32_t)dx);
    int32_t dy = (int32_t)(int16_t)rd16(m, obj + 0x52u) << 6;
    if ((rd8(m, obj + 0x1Cu) & 0x02u) == 0u)
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (uint32_t)dy);
    else
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - (uint32_t)dy);
}

static int bomb_or_death_check(const f3mem_t *m, uint32_t obj, int *out_action)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {
        *out_action = 1;
        return 1;
    }
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        *out_action = 2;
        return 1;
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u
        && (int16_t)rd16(m, obj + 0x14u) >= -0x000A
        && (int16_t)rd16(m, obj + 0x14u) <= 0x011C) {
        *out_action = 3;
        return 1;
    }
    return 0;
}

static void handle_action(const f3mem_t *m, f3regs_t *r, int action)
{
    uint32_t obj = r->a[0];
    if (action == 3) {                                   /* 0x13CA06 -> 0x0CBD12 */
        wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        wr32(m, obj + 0x10u, 0x0003C664u);
        uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
        Obj_ExplodeTick(m, obj);
        r->d[0] = (uint32_t)rd16(m, obj + 0x4Cu);
        r->a[1] = rd32(m, F3_A5 - 0x0CA0u);
        return;
    }
    if (action == 2) {                                   /* 0x13C9AE */
        uint32_t a1 = Obj_0CBDDE(m, obj);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
    }
    /* 0x13C9BE..0x13C9F6 */
    r->d[0] = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 0x30100005u);
    Sound_PlayHooked(m, 0x0016u);
    wr32(m, obj, 0x0013C9F8u);
}

int Fn_13C6DE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int action = 0;

    if (bomb_or_death_check(m, obj, &action)) {
        handle_action(m, r, action);
        return 0;
    }

    sub_13CA14(m, obj);                                  /* 0x13C714 */
    wr16(m, obj + 0x52u, (uint16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x42u)));
    Obj_AnimFrameStep(m, obj, 0x0013CA40u);              /* 0x13C724 */
    if ((int16_t)rd16(m, 0x0013CA40u + (uint32_t)(int32_t)(int16_t)rd16(m, obj + 0x3Au)) >= 0) {
        Obj_11C32C(m, obj);                              /* 0x13C732 */
        draw_sprite_regs(m, r);                          /* 0x13C738 */

        uint16_t saved_x = rd16(m, obj + 0x14u);         /* 0x13C73E */
        wr16(m, obj + 0x14u, (uint16_t)(saved_x + 0x0050u));
        wr32(m, obj + 0x10u, 0x0003C5DCu);
        draw_sprite_regs(m, r);                          /* 0x13C750 */
        wr16(m, obj + 0x14u, saved_x);

        uint16_t saved_x2 = rd16(m, obj + 0x14u);        /* 0x13C75A */
        uint16_t saved_y2 = rd16(m, obj + 0x18u);
        wr16(m, obj + 0x14u, (uint16_t)(saved_x2 + 0x0017u));
        if ((rd8(m, obj + 0x1Cu) & 0x02u) == 0u)
            wr16(m, obj + 0x18u, (uint16_t)(saved_y2 - 0x000Au));
        else
            wr16(m, obj + 0x18u, (uint16_t)(saved_y2 + 0x001Au));

        if ((rd8(m, F3_A5 + 0x0879u) & 0x02u) == 0u)
            wr32(m, obj + 0x10u, 0x0003C654u);
        else
            wr32(m, obj + 0x10u, 0x0003C65Cu);

        draw_sprite_regs(m, r);                          /* 0x13C7A0 */
        wr16(m, obj + 0x18u, saved_y2);
        wr16(m, obj + 0x14u, saved_x2);
        r->a[1] = sub_0CBDA4(m, obj);
        wr32(m, obj, 0x0013C6DEu);                       /* 0x13C7B4 -> 0x13C6D6 */
        return 0;
    }

    /* 0x13C7B8: fall through to 0x13C7C0 */
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    if (bomb_or_death_check(m, obj, &action)) {
        handle_action(m, r, action);
        return 0;
    }

    sub_13CA14(m, obj);                                  /* 0x13C7F6 */
    wr16(m, obj + 0x52u, (uint16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x42u)));
    Obj_AnimFrameStep(m, obj, 0x0013CA54u);              /* 0x13C806 */
    if ((int16_t)rd16(m, 0x0013CA54u + (uint32_t)(int32_t)(int16_t)rd16(m, obj + 0x3Au)) >= 0) {
        Obj_11C32C(m, obj);                              /* 0x13C814 */
        uint32_t saved_10 = rd32(m, obj + 0x10u);
        wr32(m, obj + 0x10u, 0x0003C5B0u);
        draw_sprite_regs(m, r);                          /* 0x13C826 */
        wr32(m, obj + 0x10u, saved_10);

        uint16_t saved_14 = rd16(m, obj + 0x14u);
        wr16(m, obj + 0x14u, (uint16_t)(saved_14 + 0x0050u));
        draw_sprite_regs(m, r);                          /* 0x13C83A */
        wr16(m, obj + 0x14u, saved_14);
        r->a[1] = sub_0CBDA4(m, obj);
        wr32(m, obj, 0x0013C852u);                       /* 0x13C84A */
        return 0;
    }

    /* 0x13C856: fall through to 0x13C85E */
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    if (bomb_or_death_check(m, obj, &action)) {
        handle_action(m, r, action);
        return 0;
    }

    sub_13CA14(m, obj);                                  /* 0x13C894 */
    wr16(m, obj + 0x52u, (uint16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x42u)));
    Obj_11C32C(m, obj);                                  /* 0x13C8A0 */
    wr32(m, obj + 0x10u, 0x0003C5B0u);
    draw_sprite_regs(m, r);                              /* 0x13C8AE */

    {
        uint16_t saved_14 = rd16(m, obj + 0x14u);
        wr16(m, obj + 0x14u, (uint16_t)(saved_14 + 0x0050u));
        uint32_t idx = rd8(m, obj + 0x3Au);
        wr32(m, obj + 0x10u, rd32(m, 0x0013CA74u + idx * 4u));
        draw_sprite_regs(m, r);                          /* 0x13C8CE */
        wr16(m, obj + 0x14u, saved_14);
    }

    uint16_t e = (uint16_t)(rd16(m, obj + 0x3Eu) + 4u);
    if ((int16_t)e > 0x0100)
        e = 0x0100u;
    wr16(m, obj + 0x3Eu, e);
    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + e));
    if ((int8_t)rd8(m, obj + 0x3Au) >= 6)
        wr8(m, obj + 0x3Au, 0u);

    if ((int16_t)e > 0x00C0) {                           /* 0x13C902 */
        uint32_t d0, d1;
        sub_0CC2BC(m, 0x0014u, &d0, &d1);                /* 0x13C910 */
        r->d[0] = d0;
        r->d[1] = d1;
        if ((uint16_t)d0 == 0u) {
            uint32_t arg = ((rd8(m, obj + 0x1Cu) & 0x02u) != 0u)
                         ? 0x0E1B4000u : 0x0E04C000u;
            (void)Obj_Spawn(m, obj, 0x000CCD04u, 9u, 2u, arg);
            arg = (arg & 0xFFFF0000u) | (uint16_t)((uint16_t)arg + 0x2000u);
            (void)Obj_Spawn(m, obj, 0x000CCD04u, 9u, 2u, arg);
            arg = (arg & 0xFFFF0000u) | (uint16_t)((uint16_t)arg - 0x4000u);
            r->d[0] = Obj_Spawn(m, obj, 0x000CCD04u, 9u, 2u, arg);
            r->d[1] = arg;
        }
    }

    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) - 0x0020u));
    r->a[1] = sub_0CBDA4(m, obj);                        /* 0x13C992 */
    if ((int16_t)rd16(m, obj + 0x14u) <= -0x0032) {      /* 0x13C998 */
        Obj_Free(m, obj);                                /* 0x13C9F8 */
        return 0;
    }
    wr32(m, obj, 0x0013C9AAu);                           /* 0x13C9A2 */
    return 0;
}
