/* ROM 0x0C1998: zoom/scale-in-out sprite tick (A0 = obj).
 * Sets (+0x1E,+0x1F) from +0x60, places (+0x14,+0x18) = (+0x44,+0x48) +
 * (+0x60 * 0x38 << 8), steps anim table +0x4C (0x0CBD52), draws sprite
 * (0x001282), integrates (+0x44,+0x48) += (+0x50,+0x54) and (+0x50,+0x54) +=
 * (0x20, 0x200), decrements +0x60 while >= 0, else clears A5+0x8DE and frees. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

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
        r->d[1] = (r->d[1] & 0xFFFF0000u) | d1w;
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x0CBD52: looping {dur.w, sprdef.l} animation step. */
static void sub_0CBD52(const f3mem_t *m, uint32_t obj, uint32_t tbl)
{
    for (;;) {
        uint16_t off = rd16(m, obj + 0x3Au);
        int16_t dur = (int16_t)rd16(m, tbl + (uint32_t)(int32_t)(int16_t)off);
        if (dur < 0) {
            wr16(m, obj + 0x3Au, 0u);
            continue;
        }
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u);
        wr16(m, obj + 0x3Eu, cnt);
        if ((int16_t)cnt <= dur) {
            wr32(m, obj + 0x10u, rd32(m, tbl + (uint32_t)(int32_t)(int16_t)off + 2u));
            return;
        }
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x3Au, (uint16_t)(off + 6u));
    }
}

int Fn_0C1998(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t sc = rd16(m, obj + 0x60u);                       /* 0x0C1998 */
    wr8(m, obj + 0x1Fu, (uint8_t)sc);                         /* 0x0C199C */
    wr8(m, obj + 0x1Eu, (uint8_t)sc);                         /* 0x0C19A0 */

    uint32_t off = ((uint32_t)sc * 0x38u) << 8;               /* 0x0C19AA..0x0C19B2 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x44u) + off);         /* 0x0C19A4..0x0C19B4 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x48u) + off);         /* 0x0C19B8..0x0C19C8 */

    sub_0CBD52(m, obj, rd32(m, obj + 0x4Cu));                 /* 0x0C19CC..0x0C19D0 -> 0x0CBD52 */
    draw_sprite_regs(m, r);                                   /* 0x0C19D6 -> 0x001282 */

    wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) + rd32(m, obj + 0x50u)); /* 0x0C19DC..0x0C19E0 */
    uint32_t d0 = rd32(m, obj + 0x54u);                       /* 0x0C19E4 */
    r->d[0] = d0;
    wr32(m, obj + 0x48u, rd32(m, obj + 0x48u) + d0);          /* 0x0C19E8 */
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + 0x00000020u); /* 0x0C19EC */
    wr32(m, obj + 0x54u, d0 + 0x00000200u);                   /* 0x0C19F4 */

    int16_t next_sc = (int16_t)(rd16(m, obj + 0x60u) - 1);    /* 0x0C19FC */
    wr16(m, obj + 0x60u, (uint16_t)next_sc);
    if (next_sc >= 0) {                                       /* 0x0C1A00 */
        wr32(m, obj, 0x000C1998u);                            /* 0x0C1990 */
        return 0;
    }

    wr32(m, F3_A5 + 0x08DEu, 0u);                             /* 0x0C1A02 */
    Obj_Free(m, obj);                                         /* 0x0C1A08 -> 0x0105EC */
    return 0;
}
