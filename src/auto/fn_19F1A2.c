/* ROM 0x19F1A2: boss spark/debris sprite tick (A0 = object).
 * Steps 10-byte movement script at +0x44 (0x178982), copies (+0x50, +0x54) to
 * (+0x14, +0x18), ticks 0x179A5C animation [+0x40], centers (+0x14, +0x18) by
 * sprite {cols*8, rows*8}, draws via 0x001282, and re-arms 0x19F1A2 while
 * +0x3E >= 0 else frees via 0x0105EC. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"
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

/* Track D1 side-effect of ROM 0x178C06 inside Obj_Script10. */
static int script10_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, obj + 0x44u);
    if (rd32(m, a1) == 0x00178C06u) {
        uint16_t d0w = (uint16_t)(rd16(m, obj + 0x42u) + rd16(m, a1 + 6u));
        uint16_t d2w = (uint16_t)(rd16(m, obj + 0x20u) + rd16(m, a1 + 8u));
        int16_t mag = (int16_t)((int16_t)(uint16_t)(d0w + 0x0020u) >> 6);
        uint16_t idx = (uint16_t)(d2w >> 8);
        int16_t sin_v = (int16_t)rd16(m, 0x00027E40u + (uint32_t)idx * 2u);
        r->d[1] = (uint32_t)((int32_t)mag * (int32_t)sin_v);
    }
    return Obj_Script10(m, obj);
}

int Fn_19F1A2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if (script10_regs(m, r) < 0)                        /* 0x19F1A2 */
        return -1;

    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));         /* 0x19F1A8 */
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));         /* 0x19F1AE */

    uint16_t anim_idx = rd16(m, obj + 0x40u);           /* 0x19F1B4 */
    if (Obj_AnimTick(m, obj, anim_idx) < 0)             /* 0x19F1B8..0x19F1C2 */
        return -1;

    uint32_t spr = rd32(m, obj + 0x10u);                /* 0x19F1C4 */
    uint16_t d1w = (uint16_t)(rd16(m, spr) << 3);       /* 0x19F1C8, 0x19F1CE */
    uint16_t d0w = (uint16_t)(rd16(m, spr + 2u) << 3);  /* 0x19F1CA, 0x19F1CC */
    r->d[1] = setw(r->d[1], d1w);
    r->d[0] = setw(r->d[0], d0w);
    r->a[1] = spr + 4u;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0w)); /* 0x19F1D0 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1w)); /* 0x19F1D4 */

    draw_sprite_regs(m, r);                             /* 0x19F1D8 */

    if ((int16_t)rd16(m, obj + 0x3Eu) >= 0) {           /* 0x19F1DE..0x19F1E2 */
        wr32(m, obj, 0x0019F1A2u);                      /* 0x19F19A */
        return 0;
    }

    Obj_Free(m, obj);                                   /* 0x19F1E4..0x19F1EC */
    return 0;                                           /* 0x19F1F0 */
}
