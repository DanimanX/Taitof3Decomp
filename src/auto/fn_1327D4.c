/* ROM 0x1327D4: multi-segment zoomed/reflected ribbon/beam tick (A0 = object).
 * Subtracts 0x60000 from +0x14.l, draws head and up to 16 trailing zoomed
 * segments with vertical mirror reflections via 0x1327FE/0x1328B8, then adds
 * 0x800 to +0x30.w; re-arms 0x1327D4 while no carry, else frees self via
 * Obj_Free (0x0105EC). */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
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

/* ROM 0x1328B8: draw vertically-flipped shadow/reflection of current segment. */
static void sub_1328B8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t save_y = rd16(m, obj + 0x18u);                         /* 0x1328B8 */
    uint16_t save_z = rd16(m, obj + 0x1Eu);                         /* 0x1328BC */

    int16_t d0w = (int16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x0100u); /* 0x1328C0..0x1328C6 */
    int16_t d1w = d0w;                                              /* 0x1328CA */
    int32_t d0l = ((int32_t)d0w * 3) >> 4;                          /* 0x1328CC..0x1328D0 */
    wr16(m, obj + 0x18u, (uint16_t)(save_y - (uint16_t)d0l));       /* 0x1328D2 */
    d1w = (int16_t)((d1w >> 3) + 0x0100);                           /* 0x1328D6..0x1328D8 */
    wr8(m, obj + 0x1Eu, (uint8_t)d1w);                              /* 0x1328DC */
    wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) | 0x02u));    /* 0x1328E0 */

    r->d[0] = (uint32_t)d0l;
    r->d[1] = setw(r->d[1], (uint16_t)d1w);
    draw_sprite_regs(m, r);                                         /* 0x1328E6 -> 0x001282 */

    wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) & ~0x02u));   /* 0x1328EC */
    wr16(m, obj + 0x1Eu, save_z);                                   /* 0x1328F2 */
    wr16(m, obj + 0x18u, save_y);                                   /* 0x1328F6 */
}

/* ROM 0x1327FE: draw head + trailing zoomed segments. */
static void sub_1327FE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t save_x = rd32(m, obj + 0x14u);                         /* 0x1327FE */
    uint32_t save_y = rd32(m, obj + 0x18u);                         /* 0x132802 */

    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 0x0080u)); /* 0x132806 */
    wr8(m, obj + 0x3Au, (uint8_t)(rd8(m, obj + 0x3Au) & 0x03u));    /* 0x13280C */
    wr16(m, obj + 0x1Eu, 0u);                                       /* 0x132812 */

    uint16_t d0w = rd8(m, obj + 0x30u);                             /* 0x132816..0x132818 */
    wr8(m, obj + 0x1Eu, (uint8_t)d0w);                              /* 0x13281C */
    d0w = (uint16_t)(((int16_t)d0w >> 4) * 3);                      /* 0x132820..0x132822 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0w));   /* 0x132826 */

    int8_t idx = (int8_t)rd8(m, obj + 0x3Au);                        /* 0x13282E..0x132832 */
    wr32(m, obj + 0x10u, rd32(m, 0x001328FCu + (uint32_t)(int32_t)idx * 4u)); /* 0x132834 */
    draw_sprite_regs(m, r);                                         /* 0x13283A */
    sub_1328B8(m, r);                                               /* 0x132840 */

    int16_t z_off = (int16_t)((uint16_t)rd8(m, obj + 0x2Cu) - 0x0100u); /* 0x132844..0x13284E */
    wr8(m, obj + 0x1Fu, rd8(m, obj + 0x2Cu));                       /* 0x13284A */
    uint32_t dx = (uint32_t)((int32_t)z_off * (1 << 13)) + 0x00008000u; /* 0x132852..0x132858 */
    uint32_t tbl2 = 0x0013290Cu;                                    /* 0x13285E */
    uint16_t d1w = (uint16_t)(int16_t)(int8_t)rd8(m, obj + 0x3Au);  /* 0x132862..0x132866 */
    uint32_t saved_d0 = dx;
    uint32_t saved_d1 = setw(r->d[1], d1w);
    uint32_t saved_a1 = tbl2;

    for (int16_t d2w = 0x0F; d2w >= 0; d2w--) {                     /* 0x132868..0x13289C */
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + saved_d0);      /* 0x13286C */
        wr32(m, obj + 0x10u, rd32(m, tbl2 + (uint32_t)(int32_t)(int16_t)d1w * 4u)); /* 0x132870 */

        draw_sprite_regs(m, r);                                     /* 0x13287A */
        sub_1328B8(m, r);                                           /* 0x132880 */

        uint8_t old_z = rd8(m, obj + 0x1Eu);                        /* 0x132888 */
        uint8_t new_z = (uint8_t)(old_z + 0x10u);
        wr8(m, obj + 0x1Eu, new_z);
        if (new_z < old_z)                                          /* 0x13288E bcs 0x1328A0 */
            break;

        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 3u)); /* 0x132892 */
        d1w = (uint16_t)((d1w + 1u) & 0x0003u);                     /* 0x132896..0x132898 */
        saved_d1 = setw(saved_d1, d1w);
    }

    r->d[0] = saved_d0;
    r->d[1] = saved_d1;
    r->a[1] = saved_a1;

    uint16_t w2c = rd16(m, obj + 0x2Cu);                            /* 0x1328A0 */
    wr16(m, obj + 0x2Cu, (uint16_t)(w2c - 0x0200u));
    if (w2c <= 0x0200u)                                             /* 0x1328A6 bhi 0x1328AE */
        wr16(m, obj + 0x2Cu, 0u);                                   /* 0x1328AA */

    wr32(m, obj + 0x18u, save_y);                                   /* 0x1328AE */
    wr32(m, obj + 0x14u, save_x);                                   /* 0x1328B2 */
}

int Fn_1327D4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - 0x00060000u);       /* 0x1327D4 */
    sub_1327FE(m, r);                                               /* 0x1327E4 */

    uint16_t old30 = rd16(m, obj + 0x30u);                          /* 0x1327E8 */
    uint16_t new30 = (uint16_t)(old30 + 0x0800u);
    wr16(m, obj + 0x30u, new30);
    if (new30 >= old30) {                                           /* 0x1327EE bcc 0x1327CC */
        wr32(m, obj, 0x001327D4u);                                  /* 0x1327CC */
        return 0;
    }

    Obj_Free(m, obj);                                               /* 0x1327F0..0x1327F8 */
    return 0;
}
