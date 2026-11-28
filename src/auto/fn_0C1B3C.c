/* ROM 0x0C1B3C: (recipe capture fnk_, work/zcap/recipes15.tsv): object tick that subtracts
 * 0x4000 from the long at +0x18, draws the sprite (0x001282) and re-arms
 * itself (0x0C1B3C). Its only capture pair spans ~0x2A6 frames, so
 * tests/test_fn.c skips it as VBlank-interrupted and the routine cannot be
 * verified against it (empty body and full body are indistinguishable to a
 * runner that rejects every pair); it is translated from the bytes. */
#include "fn_auto.h"
#include "../video/sprite.h"

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values
 * (same helper as src/auto/fn_0C12AA.c). */
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

int Fn_0C1B3C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - 0x4000u); /* 0x0C1B3C subi.l #0x4000 */
    draw_sprite_regs(m, r);                                /* 0x0C1B44 jsr 0x001282 */
    wr32(m, obj, 0x000C1B3Cu);                             /* 0x0C1B34 re-arm */
    return 0;
}
