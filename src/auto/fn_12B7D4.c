/* ROM 0x12B7D4: ship shield/beam-aura overlay tick (A0 = object, A0+0x08 = parent).
 * Frees itself (clearing A5+0x30EE bits 5..6 from MSB) when A5+0x30DA < 0x1F4 or
 * parent+0x59 != 0. Otherwise drives the charge/discharge animation table
 * 0x12B8D6 based on A5+0x30EE bit 4 (from MSB), playing sound 0x1B at peak,
 * aligns with parent + offset via 0x12B48C + (0x3F, 0x22), draws sprite
 * 0x42F5C with grandparent palette, and re-arms 0x12B7D4. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
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

/* ROM 0x12B48C: adjust (+0x18, +0x14) by sprite header dimensions * 8 - (+0x52, +0x50). */
static void sub_12B48C(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t h = (uint16_t)((rd16(m, a1) << 3) - rd16(m, obj + 0x52u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - h));
    uint16_t w = (uint16_t)((rd16(m, a1 + 2u) << 3) - rd16(m, obj + 0x50u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - w));
}

int Fn_12B7D4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* 0x12B7D4..0x12B7DA */
    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) < 0x01F4)
        goto free_self;

    /* 0x12B7DE..0x12B7E6 */
    r->a[1] = rd32(m, obj + 0x08u);
    if (rd8(m, r->a[1] + 0x59u) != 0u)
        goto free_self;

    /* 0x12B7EA..0x12B800: bftst (0x30ee,A5){4:1} = 0x08 */
    if ((rd8(m, F3_A5 + 0x30EEu) & 0x08u) == 0u && rd8(m, obj + 0x34u) == 0u) {
        wr32(m, obj + 0x10u, 0u);                           /* 0x12B7FC */
    } else {
        /* 0x12B804 */
        Obj_AnimFrameStep(m, obj, 0x0012B8D6u);             /* 0x12B808 */
        int16_t dur = (int16_t)rd16(m, 0x0012B8D6u + (uint32_t)(int32_t)(int16_t)rd16(m, obj + 0x3Au));
        if (dur < 0) {                                      /* 0x12B816 */
            wr8(m, obj + 0x34u, 0u);                        /* 0x12B84C */
            wr16(m, obj + 0x3Au, 0u);                       /* 0x12B850 */
            wr16(m, obj + 0x3Eu, 0u);                       /* 0x12B854 */
        } else if (dur == 0) {                              /* 0x12B81A */
            if (rd8(m, obj + 0x34u) == 0u) {
                Sound_PlayHooked(m, 0x001Bu);               /* 0x12B822..0x12B832 */
                wr8(m, obj + 0x34u, 0xFFu);                 /* 0x12B836 */
            }
            if ((rd8(m, F3_A5 + 0x30EEu) & 0x08u) == 0u)    /* 0x12B83A..0x12B840 */
                wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 2u)); /* 0x12B844 */
        }
    }

    /* 0x12B858..0x12B8B8 */
    {
        uint32_t saved_10 = rd32(m, obj + 0x10u);
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr32(m, obj + 0x10u, rd32(m, a1 + 0x10u));
        wr16(m, obj + 0x14u, rd16(m, a1 + 0x14u));
        wr16(m, obj + 0x18u, rd16(m, a1 + 0x18u));
        wr16(m, obj + 0x50u, rd16(m, a1 + 0x50u));
        wr16(m, obj + 0x52u, rd16(m, a1 + 0x52u));
        sub_12B48C(m, obj);                                 /* 0x12B87E */

        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)(saved_10 >> 16) + 0x003Fu));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)saved_10 + 0x0022u));
        wr32(m, obj + 0x10u, 0x00042F5Cu);                  /* 0x12B898 */
        a1 = rd32(m, obj + 0x08u);                          /* 0x12B8A0 */
        a1 = rd32(m, a1 + 0x08u);                           /* 0x12B8A4 */
        wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));            /* 0x12B8A8 */
        draw_sprite_regs(m, r);                             /* 0x12B8AE */
        wr32(m, obj + 0x10u, saved_10);                     /* 0x12B8B4 */
    }
    wr32(m, obj, 0x0012B7D4u);                              /* 0x12B8B8 -> 0x12B7CC */
    return 0;

free_self:
    /* 0x12B8BC..0x12B8D4: bfclr {5:1} (0x04) and {6:1} (0x02) on A5+0x30EE */
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & ~0x06u));
    Obj_Free(m, obj);                                       /* 0x12B8CA */
    return 0;
}
