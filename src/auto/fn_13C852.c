/* ROM 0x13C852: multi-part enemy tick (A0 = object).
 * Branches to 0x13C7C0: handles A5-0x144B bit 4 / HP <= 0 (via 0x0CBDDE score
 * relay + spawn 0x13E4EE + Sound_PlayHooked 0x16 + arm 0x13C9F8), bomb hit
 * (A5+0x1D58 bit 1 -> sprite 0x3C664 + 0x0CBD12 -> 0x0CBB2C), otherwise
 * moves via 0x13CA14, adds +0x42 to +0x52, steps animation 0x13CA54 via
 * 0x0CBD24: while duration >= 0, updates colour (0x11C32C), draws body
 * (0x3C5B0) and main sprite at +0x50 X offset, adds to collision lists
 * (0x0CBDA4), and re-arms 0x13C852. When 0x13CA54 ends (< 0), clears
 * +0x3A/+0x3E and falls into phase 0x13C85E: draws animated wing/turret
 * (0x13CA74[+0x3A byte]), ramps +0x3E up to 0x100, wraps +0x3A mod 6,
 * fires 3-way 0x0CCD04 spread when +0x3E > 0xC0 and 0x0CC2BC(0x14) == 0,
 * decelerates +0x50 by 0x20, adds to collision lists, and frees at X <= -0x32
 * or arms 0x13C9AA. */
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

/* ROM 0x0CBDA4: append obj to list1 (A5+0x8FE) and list3 (A5+0x906). */
static void add_coll_0CBDA4(const f3mem_t *m, uint32_t obj, uint32_t *a1_io)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    a1 += 4u;
    wr32(m, F3_A5 + 0x0906u, a1);
    *a1_io = a1;
}

/* ROM 0x0CC2BC: difficulty-scaled fire rate modulo check. */
static void fire_rate_0CC2BC(const f3mem_t *m, uint32_t *d0_io, uint32_t *d1_io)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = ((uint32_t)(uint16_t)*d0_io * (uint32_t)d1w) << 8;
    d1 = (d1 << 16) | (d1 >> 16);
    if ((uint16_t)d1 == 0u)
        d1 = 1u;
    uint16_t div = (uint16_t)d1;
    uint32_t dividend = (uint32_t)rd16(m, F3_A5 + 0x0878u);
    uint32_t quot = dividend / (uint32_t)div;
    uint32_t d0 = dividend;
    if (quot <= 0xFFFFu) {
        uint16_t rem = (uint16_t)(dividend % (uint32_t)div);
        d0 = ((uint32_t)(uint16_t)quot << 16) | (uint32_t)rem;
    }
    *d0_io = d0;
    *d1_io = d1;
}

/* ROM 0x13CA14: advance (+0x14, +0x18) by (+0x50, +/-0x52) << 6. */
static void move_vel_13CA14(const f3mem_t *m, uint32_t obj, uint32_t *d0_io)
{
    int32_t dx = (int32_t)(int16_t)rd16(m, obj + 0x50u) << 6;
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (uint32_t)dx);
    int32_t dy = (int32_t)(int16_t)rd16(m, obj + 0x52u) << 6;
    if ((rd8(m, obj + 0x1Cu) & 0x02u) == 0u)
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (uint32_t)dy);
    else
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - (uint32_t)dy);
    *d0_io = (uint32_t)dy;
}

int Fn_13C852(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)         /* 0x13C7C0 */
        goto death_spawn;

    if ((int32_t)rd32(m, obj + 0x24u) <= 0)               /* 0x13C7CA */
        goto hp_death;

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {        /* 0x13C7D2 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));     /* 0x13C7EA */
            goto bomb_explode;
        }
    }

    move_vel_13CA14(m, obj, &d0);                         /* 0x13C7F6 */
    d0 = setw(d0, rd16(m, obj + 0x42u));                  /* 0x13C7FA */
    wr16(m, obj + 0x52u, (uint16_t)(rd16(m, obj + 0x52u) + (uint16_t)d0)); /* 0x13C7FE */

    a1 = 0x0013CA54u;                                     /* 0x13C802 */
    Obj_AnimFrameStep(m, obj, a1);                        /* 0x13C806 */
    d0 = setw(d0, rd16(m, obj + 0x3Au));

    if ((int16_t)rd16(m, a1 + (int16_t)(uint16_t)d0) >= 0) { /* 0x13C80C */
        Obj_11C32C(m, obj);                               /* 0x13C814 */
        uint32_t saved_10 = rd32(m, obj + 0x10u);         /* 0x13C81A */
        wr32(m, obj + 0x10u, 0x0003C5B0u);                /* 0x13C81E */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        draw_sprite_regs(m, r);                           /* 0x13C826 */
        wr32(m, obj + 0x10u, saved_10);                   /* 0x13C82C */
        uint16_t saved_14 = rd16(m, obj + 0x14u);         /* 0x13C830 */
        wr16(m, obj + 0x14u, (uint16_t)(saved_14 + 0x0050u)); /* 0x13C834 */
        draw_sprite_regs(m, r);                           /* 0x13C83A */
        wr16(m, obj + 0x14u, saved_14);                   /* 0x13C840 */
        add_coll_0CBDA4(m, obj, &r->a[1]);                /* 0x13C844 */
        wr32(m, obj, 0x0013C852u);                        /* 0x13C84A */
        return 0;
    }

    wr16(m, obj + 0x3Au, 0u);                             /* 0x13C856 */
    wr16(m, obj + 0x3Eu, 0u);                             /* 0x13C85A */

    /* Phase 0x13C85E */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)          /* 0x13C85E */
        goto death_spawn;

    if ((int32_t)rd32(m, obj + 0x24u) <= 0)               /* 0x13C868 */
        goto hp_death;

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {        /* 0x13C870 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));     /* 0x13C888 */
            goto bomb_explode;
        }
    }

    move_vel_13CA14(m, obj, &d0);                         /* 0x13C894 */
    d0 = setw(d0, rd16(m, obj + 0x42u));                  /* 0x13C898 */
    wr16(m, obj + 0x52u, (uint16_t)(rd16(m, obj + 0x52u) + (uint16_t)d0)); /* 0x13C89C */
    Obj_11C32C(m, obj);                                   /* 0x13C8A0 */
    wr32(m, obj + 0x10u, 0x0003C5B0u);                    /* 0x13C8A6 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    draw_sprite_regs(m, r);                               /* 0x13C8AE */

    {
        uint16_t saved_14 = rd16(m, obj + 0x14u);         /* 0x13C8B4 */
        wr16(m, obj + 0x14u, (uint16_t)(saved_14 + 0x0050u)); /* 0x13C8B8 */
        uint32_t idx = (uint32_t)rd8(m, obj + 0x3Au);     /* 0x13C8C2 */
        wr32(m, obj + 0x10u, rd32(m, 0x0013CA74u + idx * 4u)); /* 0x13C8BE..0x13C8C8 */
        draw_sprite_regs(m, r);                           /* 0x13C8CE */
        wr16(m, obj + 0x14u, saved_14);                   /* 0x13C8D4 */
    }
    d0 = r->d[0];
    d1 = r->d[1];
    a1 = r->a[1];

    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + 4u)); /* 0x13C8D8 */
    if ((int16_t)rd16(m, obj + 0x3Eu) > 0x0100)           /* 0x13C8DC */
        wr16(m, obj + 0x3Eu, 0x0100u);                    /* 0x13C8E6 */

    d0 = setw(d0, rd16(m, obj + 0x3Eu));                  /* 0x13C8EC */
    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + (uint16_t)d0)); /* 0x13C8F0 */
    if ((int8_t)rd8(m, obj + 0x3Au) >= 6)                 /* 0x13C8F4 */
        wr8(m, obj + 0x3Au, 0u);                          /* 0x13C8FE */

    if ((int16_t)rd16(m, obj + 0x3Eu) > 0x00C0) {         /* 0x13C902 */
        d0 = setw(d0, 0x0014u);                           /* 0x13C90C */
        fire_rate_0CC2BC(m, &d0, &d1);                    /* 0x13C910 */
        if ((uint16_t)d0 == 0u) {                         /* 0x13C916 */
            d1 = ((rd8(m, obj + 0x1Cu) & 0x02u) == 0u)    /* 0x13C91C */
                 ? 0x0E04C000u : 0x0E1B4000u;             /* 0x13C926 / 0x13C930 */
            (void)Obj_Spawn(m, obj, 0x000CCD04u, 9u, 2u, d1); /* 0x13C936..0x13C946 */
            d1 = (d1 & 0xFFFF0000u) | (uint16_t)((uint16_t)d1 + 0x2000u); /* 0x13C950 */
            (void)Obj_Spawn(m, obj, 0x000CCD04u, 9u, 2u, d1); /* 0x13C954..0x13C964 */
            d1 = (d1 & 0xFFFF0000u) | (uint16_t)((uint16_t)d1 - 0x4000u); /* 0x13C96E */
            d0 = Obj_Spawn(m, obj, 0x000CCD04u, 9u, 2u, d1);  /* 0x13C972..0x13C982 */
        }
    }

    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) - 0x0020u)); /* 0x13C98C */
    add_coll_0CBDA4(m, obj, &a1);                         /* 0x13C992 */
    if ((int16_t)rd16(m, obj + 0x14u) <= -0x0032)         /* 0x13C998 */
        Obj_Free(m, obj);                                 /* 0x13C9F8 */
    else
        wr32(m, obj, 0x0013C9AAu);                        /* 0x13C9A2 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;

bomb_explode:
    wr32(m, obj + 0x10u, 0x0003C664u);                    /* 0x13CA06 */
    a1 = rd32(m, F3_A5 + 0x1D50u);                        /* 0x0CBD12 */
    d0 = (uint32_t)rd16(m, obj + 0x4Cu);                  /* 0x0CBD16..0x0CBD18 */
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);        /* 0x0CBD1C */
    Obj_ExplodeTick(m, obj);                              /* 0x0CBD20 -> 0x0CBB2C */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;

hp_death:
    a1 = Obj_0CBDDE(m, obj);                              /* 0x13C9AE */
    d0 = (uint32_t)rd16(m, obj + 0x4Cu);                  /* 0x13C9B4..0x13C9B6 */
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);        /* 0x13C9BA */

death_spawn:
    d0 = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 0x30100005u); /* 0x13C9BE..0x13C9D2 */
    Sound_PlayHooked(m, 0x0016u);                         /* 0x13C9DC..0x13C9E6 */
    wr32(m, obj, 0x0013C9F8u);                            /* 0x13C9F0 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
