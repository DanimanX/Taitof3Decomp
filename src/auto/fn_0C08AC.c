/* ROM 0x0C08AC: (recipe capture fnk_, work/zcap/recipes15.tsv): animation-driven effect object
 * tick (A0 = object).
 * Steps the {dur.w, sprdef.l} animation table at +0x5C via 0x0CBD24, draws the
 * sprite (0x001282), decrements +0x60 and re-arms 0x0C08AC while it stays >= 0;
 * otherwise it clears the long at A5+0x8CE+(zero-extended word +0x58) and frees
 * the object through 0x0105EC.
 * The capture holds a single pair whose post state was sampled 678 frames
 * after the entry (the runner skips it as VBlank-interrupted), so this
 * translation is checked against the bytes only. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x0CBD24 with D0/D1 register tracking. */
static void sub_0CBD24(const f3mem_t *m, uint32_t obj, uint32_t tbl,
                       uint32_t *d0_io, uint32_t *d1_io)
{
    for (;;) {
        uint16_t off = rd16(m, obj + 0x3Au);
        *d0_io = (*d0_io & 0xFFFF0000u) | off;
        int16_t dur = (int16_t)rd16(m, tbl + (uint32_t)(int32_t)(int16_t)off);
        if (dur <= 0)
            return;
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u);
        wr16(m, obj + 0x3Eu, cnt);
        *d1_io = (*d1_io & 0xFFFF0000u) | cnt;
        if ((int16_t)cnt <= dur) {
            wr32(m, obj + 0x10u, rd32(m, tbl + (uint32_t)(int32_t)(int16_t)off + 2u));
            return;
        }
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x3Au, (uint16_t)(off + 6u));
    }
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
        r->d[1] = (r->d[1] & 0xFFFF0000u) | d1w;
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

int Fn_0C08AC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    r->a[1] = rd32(m, obj + 0x5Cu);                          /* 0x0C08AC movea.l */
    sub_0CBD24(m, obj, r->a[1], &r->d[0], &r->d[1]);         /* 0x0C08B0 */
    draw_sprite_regs(m, r);                                  /* 0x0C08B6 -> 0x001282 */

    int16_t rem = (int16_t)(rd16(m, obj + 0x60u) - 1u);      /* 0x0C08BC */
    wr16(m, obj + 0x60u, (uint16_t)rem);
    if (rem >= 0) {                                          /* 0x0C08C0 bge */
        wr32(m, obj, 0x000C08ACu);                           /* 0x0C08A4 re-arm */
    } else {
        /* move.w (0x58,A0),D0w: the index is the zero-extended word */
        r->d[0] = (uint32_t)rd16(m, obj + 0x58u);            /* 0x0C08C2 */
        wr32(m, F3_A5 + 0x08CEu + (uint16_t)r->d[0], 0u);    /* 0x0C08C6 clr.l */
        Obj_Free(m, obj);                                    /* 0x0C08CE -> 0x0105EC */
    }
    return 0;
}
