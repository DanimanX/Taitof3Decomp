/* ROM 0x10E494: 3D-orbiting enemy/hazard spawner & tick (A0 = object).
 * Dies via 0x0CEE74 if bomb flag (A5-0x144B bit 5) is set. Otherwise scrolls
 * (+0x3A, +0x3E) by -(A5+0x4224, A5+0x4228), copies to (+0x14, +0x18), and
 * dispatches state +0x06.b through jump table 0x10E4DE:
 *   1 (0x10E4EA): counts down +0x23.b; on 0 spawns 0x10E806 (group 9, 2),
 *                 plays hooked sound 0x27, and advances state to 2.
 *   2 (0x10E52A): steps 3D orbit via 0x0CF98A; if swapped D1w >= 0 clears +0x06
 *                 (dying via 0x0CEE74), else adds D1w to +0x18, plays hooked
 *                 sound 0x24 when (+0x0C)->+0x08 == 0xA6, registers in collision
 *                 lists (0x0D0DC2), and draws sprite (0x001282). */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj,
                             uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    *a1_io = rd32(m, F3_A5 - 0x0CA0u);

    if (spr_hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        *d0_io = d0;
        *d1_io = setw(*d1_io, d1w);
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        *d0_io = 0x0000FFFFu;
        *d1_io = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x0D25E4: multi-step vector scale (0x0D1EDC/0x0D2044 + 0x0D263C/0x0D27A4). */
static void sub_0D25E4(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint16_t d4w = 0u;
    while (d1w >= 0x0032u) {
        d4w = (uint16_t)(d4w + 1u);
        d1w = (uint16_t)(d1w - 0x0032u);
    }
    int32_t idx = (int32_t)(int16_t)d0w;
    uint32_t d0 = rd32(m, (uint32_t)(0x000D1EDCu + idx * 4)) * (uint32_t)d1w;
    uint32_t d1 = rd32(m, (uint32_t)(0x000D2044u + idx * 4)) * (uint32_t)d1w;
    uint32_t hi0 = rd32(m, (uint32_t)(0x000D263Cu + idx * 4)) * (uint32_t)d4w;
    uint32_t hi1 = rd32(m, (uint32_t)(0x000D27A4u + idx * 4)) * (uint32_t)d4w;
    *out_d0 = d0 + hi0;
    *out_d1 = d1 + hi1;
}

static void wrap360(const f3mem_t *m, uint32_t a, uint32_t step)
{
    uint32_t v = rd32(m, a) + rd32(m, step);
    wr32(m, a, v);
    if ((int32_t)v < 0)
        wr16(m, a, (uint16_t)(rd16(m, a) + 0x0168u));
    else if (rd16(m, a) >= 0x0168u)
        wr16(m, a, (uint16_t)(rd16(m, a) - 0x0168u));
}

/* ROM 0x0CF98A: orbit offset (D0, D1) from motion block at +0x0C. */
static void sub_0CF98A(const f3mem_t *m, uint32_t obj,
                       uint32_t *out_d0, uint32_t *out_d1, uint32_t *out_a1)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);                  /* 0x0CF98A */
    const uint16_t d6w = 0x0168u;

    wrap360(m, a1 + 0x08u, a1 + 0x0Cu);                  /* 0x0CF992..0x0CF9AE */
    uint32_t v = rd32(m, a1 + 0x10u) + rd32(m, a1 + 0x14u); /* 0x0CF9B2..0x0CF9B6 */
    wr32(m, a1 + 0x10u, v);
    if ((int32_t)v < 0)
        wr32(m, a1 + 0x10u, 0u);
    else if (rd16(m, a1 + 0x10u) > 0x0FFFu)
        wr16(m, a1 + 0x10u, 0x0FFFu);
    wrap360(m, a1 + 0x18u, a1 + 0x1Cu);                  /* 0x0CF9D6..0x0CF9F2 */
    wrap360(m, a1 + 0x20u, a1 + 0x24u);                  /* 0x0CF9F6..0x0CFA12 */

    uint16_t d0w = rd16(m, a1 + 0x08u);                  /* 0x0CFA16 */
    uint16_t d1w = rd16(m, a1 + 0x10u);                  /* 0x0CFA1A */
    uint16_t d2w = d0w;
    uint16_t d3w = d1w;
    uint32_t d0, d1;
    sub_0D25E4(m, d0w, d1w, &d0, &d1);                   /* 0x0CFA22 */
    uint32_t d4 = d0, d5 = d1;

    d2w = (uint16_t)(d2w - rd16(m, a1 + 0x18u));         /* 0x0CFA2C */
    if ((int16_t)d2w < 0)
        d2w = (uint16_t)(d2w + d6w);
    sub_0D25E4(m, d2w, d3w, &d0, &d1);                   /* 0x0CFA3A */

    d0w = rd16(m, a1 + 0x20u);                           /* 0x0CFA40 */
    d1w = (uint16_t)(d1 >> 16);                          /* 0x0CFA44 swap D1 */
    d3w = 0x005Au;
    if (d2w < 0x00B4u) {                                 /* 0x0CFA48..0x0CFA4C */
        d1w = (uint16_t)(0u - d1w);
        d3w = (uint16_t)(d3w + 0x00B4u);
    }
    d2w = d1w;
    sub_0D25E4(m, d0w, d1w, &d0, &d1);                   /* 0x0CFA58 */

    d0w = (uint16_t)(rd16(m, a1 + 0x18u) + d3w);         /* 0x0CFA5E..0x0CFA62 */
    if (d0w >= d6w)
        d0w = (uint16_t)(d0w - d6w);
    d1w = (uint16_t)((uint16_t)(d1 >> 16) + d2w);        /* 0x0CFA6C..0x0CFA6E */
    sub_0D25E4(m, d0w, d1w, &d0, &d1);                   /* 0x0CFA70 */

    *out_d0 = d0 + d4;                                   /* 0x0CFA76 */
    *out_d1 = d1 + d5;                                   /* 0x0CFA78 */
    *out_a1 = a1;
}

int Fn_10E494(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {       /* 0x10E494..0x10E49A */
        wr8(m, obj + 0x06u, 0u);                         /* 0x10E4D4 */
        Obj_StartDying(m, obj);                          /* 0x10E4D8 */
        return 0;
    }

    d0 = rd32(m, F3_A5 + 0x4224u);                       /* 0x10E49E */
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - d0);     /* 0x10E4A2 */
    d0 = rd32(m, F3_A5 + 0x4228u);                       /* 0x10E4A6 */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);     /* 0x10E4AA */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));          /* 0x10E4AE */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));          /* 0x10E4B4 */

    uint8_t st = rd8(m, obj + 0x06u);                    /* 0x10E4BA..0x10E4BC */
    d0 = (d0 & 0xFFFF0000u) | st;
    if (st == 0u || st > 2u)
        return -1;

    if (st == 1u) {                                      /* 0x10E4EA */
        uint8_t cnt = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, cnt);
        if (cnt == 0u) {                                 /* 0x10E4EE */
            d0 = Obj_Spawn(m, obj, 0x0010E806u, 9u, 2u, 0u); /* 0x10E4F2..0x10E50A */
            Sound_PlayHooked(m, 0x0027u);                /* 0x10E50E..0x10E51E */
            wr8(m, obj + 0x06u, 2u);                     /* 0x10E522 */
        }
    } else {                                             /* 0x10E52A */
        sub_0CF98A(m, obj, &d0, &d1, &a1);
        d1 = (d1 << 16) | (d1 >> 16);                    /* 0x10E530 swap D1 */
        if ((int16_t)(uint16_t)d1 >= 0) {                /* 0x10E532..0x10E534 */
            wr8(m, obj + 0x06u, 0u);                     /* 0x10E56C */
        } else {
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d1)); /* 0x10E538 */
            a1 = rd32(m, obj + 0x0Cu);                   /* 0x10E53C */
            if (rd16(m, a1 + 0x08u) == 0x00A6u)          /* 0x10E540..0x10E546 */
                Sound_PlayHooked(m, 0x0024u);            /* 0x10E54A..0x10E55A */
            Obj_0D0DC2(m, obj);                          /* 0x10E55E */
            draw_sprite_regs(m, obj, &d0, &d1, &a1);     /* 0x10E564 */
        }
    }

    if (rd8(m, obj + 0x06u) == 0u)                       /* 0x10E4C8..0x10E4CC */
        Obj_StartDying(m, obj);

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
