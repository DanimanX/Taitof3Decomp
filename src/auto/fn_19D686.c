/* ROM 0x19D686: boss core/part tick (A0 = object).
 * If A5+0x4B38 == -1, sets +0x06 = 2 and arms 0x17A85C (0x17A84C). Otherwise
 * runs 10-byte script (0x178982), copies (+0x50, +0x54) -> (+0x14, +0x18),
 * ticks anim +0x40 (0x179A5C), centers sprite by +0x10 dims*8 - 4, draws
 * sprite (0x001282), and while A5+0x4B3A MSB is set re-arms 0x19D686; else
 * installs script 0x19D7B0 and arms 0x19D6FA. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_script10.h"
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

/* ROM 0x1798D4 (index 10) + Obj_AnimTick for other translated indices. */
static int anim_tick_local(const f3mem_t *m, uint32_t obj, uint16_t idx)
{
    if (idx == 10u) {
        uint8_t ang = rd8(m, obj + 0x20u);
        uint16_t d0 = (uint16_t)((uint8_t)(ang + 4u) >> 3);
        uint32_t a1 = rd32(m, obj + 0x3Au) + ((uint32_t)d0 * 6u & 0xFFFFu);
        wr32(m, obj + 0x10u, rd32(m, a1));
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1 + 4u));
        return 0;
    }
    return Obj_AnimTick(m, obj, idx);
}

/* ROM 0x1849AA: scale D0w by difficulty nibble table at 0x1849D6. */
static uint16_t sub_1849AA(const f3mem_t *m, uint16_t d0w)
{
    uint16_t sel = (uint16_t)((rd16(m, F3_A5 + 0x087Eu) >> 4) & 0x0Fu);
    uint32_t d1 = (uint32_t)rd8(m, 0x001849D6u + sel);
    int32_t d2 = (int32_t)((uint32_t)d0w << 16);
    d2 = (d2 / 0x64) * (int32_t)d1;
    uint32_t u2 = ((uint32_t)d2 << 16) | ((uint32_t)d2 >> 16);
    return (uint16_t)(d0w + (uint16_t)u2);
}

/* ROM 0x178F3E: difficulty-scaled steer toward target (+0x4C) along 0x27E40. */
static void scr_178F3E(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    int32_t tgt = (int32_t)rd32(m, obj + 0x4Cu);         /* 0x178F3E */
    if (tgt <= 0)                                        /* 0x178F42 ble */
        return;

    uint16_t dx = (uint16_t)(rd16(m, (uint32_t)tgt + 0x14u) + 0x0018u - rd16(m, obj + 0x50u));
    uint16_t dy = (uint16_t)(rd16(m, (uint32_t)tgt + 0x18u) + 0x0010u - rd16(m, obj + 0x54u));
    uint16_t ang = Obj_Atan2(m, dx, dy);                 /* 0x178F62 -> 0x006422 */

    uint32_t a1 = rd32(m, obj + 0x44u);                  /* 0x178F6A */
    wr16(m, obj + 0x42u, sub_1849AA(m, rd16(m, a1 + 6u))); /* 0x178F6E..0x178F78 */
    uint16_t d0 = sub_1849AA(m, rd16(m, a1 + 8u));       /* 0x178F7C..0x178F80 */

    int8_t diff = (int8_t)((uint8_t)ang - rd8(m, obj + 0x20u)); /* 0x178F86 */
    if (diff == 0)                                       /* 0x178F8C */
        d0 = 0u;
    else if (diff < 0)                                   /* 0x178F90 bgt */
        d0 = (uint16_t)(0u - d0);                        /* 0x178F94 */
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + d0)); /* 0x178F9C */

    int16_t mag = (int16_t)(uint16_t)(rd16(m, obj + 0x42u) + 0x0020u) >> 6; /* 0x178FA0..0x178FA8 */
    uint16_t idx = rd16(m, obj + 0x20u) >> 8;            /* 0x178FAC..0x178FB0 */
    uint32_t base = 0x00027E40u + (uint32_t)idx * 2u;    /* 0x178FB2 */
    int32_t vx = (int32_t)mag * (int16_t)rd16(m, base + 0x80u); /* 0x178FB8 */
    int32_t vy = (int32_t)mag * (int16_t)rd16(m, base);  /* 0x178FBE */
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)vx); /* 0x178FC4 */
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)vy); /* 0x178FC8 */
    r->d[0] = (uint32_t)vx;
    r->d[1] = (uint32_t)vy;
}

/* ROM 0x178DB6: accumulate +0x42, jitter +0x20.b by 0x1794A4, step along 0x27E40. */
static void scr_178DB6(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);                  /* 0x178DBA */
    uint16_t d1w = (uint16_t)(rd16(m, obj + 0x42u) + rd16(m, a1 + 6u)); /* 0x178DB6..0x178DBE */
    uint8_t d0b = (uint8_t)(Obj_RandRange(m, rd8(m, a1 + 9u)) + rd8(m, obj + 0x20u)); /* 0x178DC2..0x178DCC */
    wr8(m, obj + 0x20u, d0b);                            /* 0x178DD0 */
    wr16(m, obj + 0x42u, d1w);                           /* 0x178DD4 */

    int16_t mag = (int16_t)(uint16_t)(d1w + 0x0020u) >> 6; /* 0x178DD8..0x178DDC */
    uint32_t base = 0x00027E40u + (uint32_t)d0b * 2u;    /* 0x178DE0..0x178DE4 */
    int32_t vx = (int32_t)mag * (int16_t)rd16(m, base + 0x80u); /* 0x178DEA */
    int32_t vy = (int32_t)mag * (int16_t)rd16(m, base);  /* 0x178DF0 */
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)vx); /* 0x178DF6 */
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)vy); /* 0x178DFA */
    r->d[0] = setw(r->d[0], d0b);
    r->d[1] = (uint32_t)vx;
}

/* ROM 0x178ECC: aim at target (+0x4C) with bitmask+offset jitter, step along 0x27E40. */
static void scr_178ECC(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    int32_t tgt = (int32_t)rd32(m, obj + 0x4Cu);
    if (tgt <= 0)
        return;

    uint16_t dx = (uint16_t)(rd16(m, (uint32_t)tgt + 0x14u) + 0x0018u - rd16(m, obj + 0x50u));
    uint16_t dy = (uint16_t)(rd16(m, (uint32_t)tgt + 0x18u) + 0x0010u - rd16(m, obj + 0x54u));
    uint16_t d0 = Obj_Atan2(m, dx, dy);

    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t d2 = rd16(m, a1 + 0x06u);
    uint16_t d1 = rd16(m, a1 + 0x08u);
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + d2));

    uint8_t ang = (uint8_t)((uint8_t)d0 & (uint8_t)d1);
    ang = (uint8_t)(ang + (uint8_t)(int8_t)(int16_t)((int16_t)d1 >> 8));
    wr8(m, obj + 0x20u, ang);

    int16_t mag = (int16_t)(uint16_t)(rd16(m, obj + 0x42u) + 0x0020u) >> 6;
    uint16_t idx = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    uint32_t base = 0x00027E40u + (uint32_t)idx * 2u;
    int32_t vx = (int32_t)mag * (int16_t)rd16(m, base + 0x80u);
    int32_t vy = (int32_t)mag * (int16_t)rd16(m, base);
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)vx);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)vy);
    r->d[0] = (uint32_t)vx;
    r->d[1] = (uint32_t)vy;
}

/* ROM 0x178982 with 0x178ECC / 0x178F3E / 0x178DB6 support + Obj_Script10 fallback. */
static int script10_step_19D686(const f3mem_t *m, f3regs_t *r, uint32_t obj)
{
    uint32_t a1;

    for (;;) {
        a1 = rd32(m, obj + 0x44u);
        uint32_t fn = rd32(m, a1);

        if (fn == 0u)
            goto loop_marker;

        if (fn == 0x00178B20u)
            Obj_Script10_SetHeading(m, obj);
        else if (fn == 0x00178ECCu)
            scr_178ECC(m, r, obj);
        else if (fn == 0x00178F3Eu)
            scr_178F3E(m, r, obj);
        else if (fn == 0x00178DB6u)
            scr_178DB6(m, r, obj);
        else
            return Obj_Script10(m, obj);

        int16_t n = (int16_t)rd16(m, obj + 0x48u);
        if (n < 0)
            return 0;
        if (n > 0) {
            n = (int16_t)(n - 1);
            wr16(m, obj + 0x48u, (uint16_t)n);
            if (n > 0)
                return 0;
            a1 = rd32(m, obj + 0x44u) + 0x0Au;
            if ((int32_t)rd32(m, a1) > 0)
                goto store;
            goto loop_marker;
        }
        a1 = rd32(m, obj + 0x44u) + 0x0Au;
        wr32(m, obj + 0x44u, a1);
        wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
    }

loop_marker: {
        int16_t c = (int16_t)rd16(m, obj + 0x4Au);
        if (c >= 0) {
            if (c == 0)
                wr16(m, obj + 0x4Au, rd16(m, a1 + 4u));
            c = (int16_t)(rd16(m, obj + 0x4Au) - 1u);
            wr16(m, obj + 0x4Au, (uint16_t)c);
            if (c == 0) {
                a1 += 0x0Au;
                goto store;
            }
        }
        a1 = rd32(m, a1 + 6u);
    }

store:
    wr32(m, obj + 0x44u, a1);
    wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
    return 0;
}

int Fn_19D686(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int16_t)rd16(m, F3_A5 + 0x4B38u) == -1) {       /* 0x19D686..0x19D68C */
        wr16(m, obj + 0x06u, 2u);                        /* 0x17A84C (0x17A852 beq folded) */
        wr32(m, obj + 0x00u, 0x0017A85Cu);               /* 0x17A854 */
        return 0;
    }

    uint16_t anim_idx = rd16(m, obj + 0x40u);
    if (anim_idx == 3u || anim_idx == 11u || anim_idx == 13u || anim_idx > 16u)
        return -1;

    if (script10_step_19D686(m, r, obj) < 0)             /* 0x19D692 -> 0x178982 */
        return -1;

    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));          /* 0x19D698 */
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));          /* 0x19D69E */

    if (anim_tick_local(m, obj, anim_idx) < 0)           /* 0x19D6A4..0x19D6B2 */
        return -1;

    {
        uint32_t a1 = rd32(m, obj + 0x10u);              /* 0x19D6B4 */
        int16_t d1w = (int16_t)(rd16(m, a1) << 3);       /* 0x19D6B8, 0x19D6BE */
        int16_t d0w = (int16_t)(rd16(m, a1 + 2u) << 3);  /* 0x19D6BA, 0x19D6BC */
        wr16(m, obj + 0x14u, (uint16_t)((int16_t)rd16(m, obj + 0x14u) - d0w)); /* 0x19D6C0 */
        wr16(m, obj + 0x18u, (uint16_t)((int16_t)rd16(m, obj + 0x18u) - d1w)); /* 0x19D6C4 */
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 4u)); /* 0x19D6C8 */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 4u)); /* 0x19D6CC */
    }

    draw_sprite_regs(m, r);                              /* 0x19D6D0 -> 0x001282 */

    if ((rd8(m, F3_A5 + 0x4B3Au) & 0x80u) != 0u) {       /* 0x19D6D6..0x19D6DC */
        wr32(m, obj + 0x00u, 0x0019D686u);               /* 0x19D67E */
        return 0;
    }

    {
        uint32_t a1 = 0x0019D7B0u;                       /* 0x19D6DE */
        r->a[1] = a1;
        wr32(m, obj + 0x44u, a1);                        /* 0x19D6E4 */
        wr16(m, obj + 0x48u, rd16(m, a1 + 4u));          /* 0x19D6E8 */
        wr16(m, obj + 0x4Au, 0u);                        /* 0x19D6EE */
        wr32(m, obj + 0x00u, 0x0019D6FAu);               /* 0x19D6F2 */
    }
    return 0;
}
