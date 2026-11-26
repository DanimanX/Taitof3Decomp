/* ROM 0x1100C6: boss/snake body segment tick (A0 = object).
 * If state (+0x06) == 0, applies velocity (+0x50/+0x54) with gravity (0x36B0),
 * culls off-screen (+0x14 < -0x40 or +0x18 > 0xE8 -> 0x0CEE74), and draws
 * (0x001282). If parent (+0x08)->+0x06 == 0, launches into ballistic debris
 * (0x0D0E3A + 0x0D2D44 -> +0x50/+0x54, +0x06 = 0). Otherwise copies +0x1D
 * from parent, syncs +0x3A/+0x3E from +0x50->+0x3A/+0x3E, reads next angle
 * from ring buffer (+0x0C) into +0x58, steps position by 0x0D1EBC and sprite
 * by 0x0CFA7C using table 0x10FC90[+0x22]. If +0x22 != 9, adds to collision
 * lists (0x0D0DC2); if +0x22 == 9, walks chain via +0x54 (when parent +0x58
 * < 0xB4) or +0x50 (when >= 0xB4) and draws each segment with 0x10FFB0
 * shadow/extra pass for +0x22 == 2. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0D1EBC: (D0, D1) = {0x0D1EDC, 0x0D2044}[d0w] * d1w (via 0x0D4F26). */
static int sub_0D1EBC(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    if (d1w > 50u)
        return -1;
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    *out_d0 = rd32(m, 0x000D1EDCu + off) * (uint32_t)d1w;
    *out_d1 = rd32(m, 0x000D2044u + off) * (uint32_t)d1w;
    return 0;
}

/* ROM 0x0D2D44: (D0, D1) = {0x0D2D64, 0x0D2ECC}[d0w] * d1w (via 0x0D4F26). */
static int sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                      uint32_t *out_d0, uint32_t *out_d1)
{
    if (d1w > 50u)
        return -1;
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    *out_d0 = rd32(m, 0x000D2D64u + off) * (uint32_t)d1w;
    *out_d1 = rd32(m, 0x000D2ECCu + off) * (uint32_t)d1w;
    return 0;
}

/* ROM 0x0CFA7C with D0/A1 register tracking. */
static void sub_0CFA7C_regs(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d0w,
                            uint32_t *d0_io, uint32_t *a1_io)
{
    int16_t off = (int16_t)rd16(m, 0x000CFA96u + (uint32_t)d0w * 2u);
    a1 = (uint32_t)((int32_t)a1 + (int32_t)off);
    uint16_t pal = rd16(m, a1);
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)pal);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
    *d0_io = setw(*d0_io, pal);
    *a1_io = a1;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj,
                             uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    *a1_io = rd32(m, F3_A5 - 0x0CA0u);

    if (hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        *d0_io = d0;
        *d1_io = setw(*d1_io, d1w);
    } else {
        uint16_t cols = (uint16_t)(hdr >> 16);
        *d0_io = 0x0000FFFFu;
        *d1_io = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x1101BA..0x110202 / 0x110228..0x110270 helper */
static void draw_seg_alt(const f3mem_t *m, uint32_t obj,
                         uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t s10 = rd32(m, obj + 0x10u);
    uint8_t s1c = rd8(m, obj + 0x1Cu);
    uint16_t s14 = rd16(m, obj + 0x14u);
    uint16_t s18 = rd16(m, obj + 0x18u);

    sub_0CFA7C_regs(m, obj, 0x0010FFB0u, rd16(m, obj + 0x58u), d0_io, a1_io);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0010u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x0018u));
    draw_sprite_regs(m, obj, d0_io, d1_io, a1_io);

    wr16(m, obj + 0x18u, s18);
    wr16(m, obj + 0x14u, s14);
    wr8(m, obj + 0x1Cu, s1c);
    wr32(m, obj + 0x10u, s10);
}

int Fn_1100C6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if (rd8(m, obj + 0x06u) == 0u)                      /* 0x1100C6..0x1100CA */
        goto ballistic;

    a1 = rd32(m, obj + 0x08u);                          /* 0x1100CE */
    if (rd8(m, a1 + 0x06u) == 0u) {                     /* 0x1100D2..0x1100D6 */
        uint16_t r1 = (uint16_t)((Obj_Next4Way(m) & 7u) + 0x1Cu); /* 0x1100DA..0x1100E8 */
        uint16_t r0 = (uint16_t)((Obj_Next4Way(m) & 0x3Fu) + 0x3Au); /* 0x1100EA..0x1100F4 */
        if (sub_0D2D44(m, r0, r1, &d0, &d1) < 0)        /* 0x1100F8 */
            return -1;
        wr32(m, obj + 0x50u, d0);                       /* 0x1100FE */
        wr32(m, obj + 0x54u, d1);                       /* 0x110102 */
        wr8(m, obj + 0x1Du, 0u);                        /* 0x110106 */
        wr8(m, obj + 0x06u, 0u);                        /* 0x11010A */
        goto ballistic;                                 /* 0x11010E */
    }

    wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));            /* 0x110112 */
    a1 = rd32(m, obj + 0x50u);                          /* 0x110118 */
    wr32(m, obj + 0x3Au, rd32(m, a1 + 0x3Au));          /* 0x11011C */
    wr32(m, obj + 0x3Eu, rd32(m, a1 + 0x3Eu));          /* 0x110122 */

    a1 = rd32(m, obj + 0x0Cu);                          /* 0x110128 */
    uint16_t idx = rd16(m, a1);                         /* 0x11012C */
    uint32_t ptr = rd32(m, a1 + 4u) + (uint32_t)(int32_t)(int16_t)idx; /* 0x11012E: ([4,A1],D0w) */
    wr16(m, obj + 0x58u, rd16(m, ptr));
    idx = (uint16_t)((idx + 2u) & 0x01FFu);             /* 0x110136..0x110138 */
    wr16(m, a1, idx);                                   /* 0x11013C */

    uint16_t d1w = (uint16_t)rd8(m, obj + 0x22u);       /* 0x11013E..0x110140 */
    a1 = rd32(m, 0x0010FC90u + (uint32_t)d1w * 4u);     /* 0x110144 */
    d1 = rd32(m, a1); a1 += 4u;                         /* 0x11014A */
    uint16_t d2w = rd16(m, a1); a1 += 2u;               /* 0x11014C */
    uint16_t d3w = rd16(m, a1); a1 += 2u;               /* 0x11014E */
    a1 = rd32(m, a1);                                   /* 0x110150 */

    sub_0CFA7C_regs(m, obj, a1, rd16(m, obj + 0x58u), &d0, &a1); /* 0x110152..0x110156 */
    if (sub_0D1EBC(m, rd16(m, obj + 0x58u), (uint16_t)d1, &d0, &d1) < 0) /* 0x11015C..0x110160 */
        return -1;
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - d0);    /* 0x110166 */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d1);    /* 0x11016A */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));         /* 0x11016E */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d2w)); /* 0x110174 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));         /* 0x110178 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d3w)); /* 0x11017E */

    if (rd8(m, obj + 0x22u) != 9u) {                    /* 0x110182..0x110188 */
        Obj_0D0DC2(m, obj);                             /* 0x11018C */
        a1 = rd32(m, F3_A5 + 0x0906u);
        goto done;
    }

    a1 = rd32(m, obj + 0x08u);                          /* 0x110194 */
    if (rd16(m, a1 + 0x58u) < 0x00B4u) {                /* 0x110198..0x11019E */
        obj = a1;                                       /* 0x1101A2 */
        draw_sprite_regs(m, obj, &d0, &d1, &a1);        /* 0x1101A4 */
        d0 = rd32(m, obj + 0x54u);                      /* 0x1101AA */
        for (;;) {
            obj = d0;                                   /* 0x1101AE */
            if (rd8(m, obj + 0x22u) == 2u)              /* 0x1101B0..0x1101B6 */
                draw_seg_alt(m, obj, &d0, &d1, &a1);    /* 0x1101BA..0x110202 */
            draw_sprite_regs(m, obj, &d0, &d1, &a1);    /* 0x110206 */
            d0 = rd32(m, obj + 0x54u);                  /* 0x11020C */
            if ((int32_t)d0 < 0)                        /* 0x110210 */
                break;
        }
        goto done;
    }

    draw_sprite_regs(m, obj, &d0, &d1, &a1);            /* 0x110214 */
    for (;;) {
        obj = rd32(m, obj + 0x50u);                     /* 0x11021A */
        if (rd8(m, obj + 0x22u) == 2u)                  /* 0x11021E..0x110224 */
            draw_seg_alt(m, obj, &d0, &d1, &a1);        /* 0x110228..0x110270 */
        draw_sprite_regs(m, obj, &d0, &d1, &a1);        /* 0x110274 */
        if (rd8(m, obj + 0x22u) == 0u)                  /* 0x11027A..0x11027E */
            break;
    }
    goto done;

ballistic:                                              /* 0x110282 */
    d0 = rd32(m, obj + 0x50u);                          /* 0x110282 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + d0);    /* 0x110286 */
    d0 = rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u); /* 0x11028A..0x11028E */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d0);    /* 0x110292 */
    if ((int16_t)rd16(m, obj + 0x14u) < -0x40 ||        /* 0x110296..0x11029C */
        (int16_t)rd16(m, obj + 0x18u) > 0x00E8) {       /* 0x1102A2..0x1102A8 */
        Obj_StartDying(m, obj);
        goto done;
    }
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + 0x36B0u); /* 0x1102AE */
    draw_sprite_regs(m, obj, &d0, &d1, &a1);            /* 0x1102B6 */

done:
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[0] = obj;
    r->a[1] = a1;
    return 0;
}
