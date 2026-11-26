/* ROM 0x164692: screen fade/mosaic ramp and stage transition tilemap setup (A0 = object).
 * Computes mosaic/fade value from |(+0x44 >> 2) - 8| and fills 0xE8 words at
 * 0x626200 + (A5-0x5D56) while preserving D0/D1/A0. When +0x44 == 0x20, clears
 * A5+0x4286, resets A5+0x4234..0x4238 to (0x14000, 0), copies scroll
 * (A5-0x5D90, A5-0x5D8C) to A5+0x4214 and A5-0x5D98, unpacks 8 tilemaps at
 * 0x612000..0x612460 (0x00523A, attr 0xDE) and 2 tilemaps at 0x610020/0x610420
 * (0x0053B2). Decrements +0x44 and re-arms 0x164692 while >= 0, else frees
 * via 0x0105EC. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x00523A (defined in src/enemy/obj_14C8F6.c). */
void Gfx_DrawTilemapAttr_00523A(const f3mem_t *m, uint32_t src, uint32_t dst,
                                uint16_t attr, uint16_t flags);

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_164692(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t d0w = (int16_t)rd16(m, obj + 0x44u);          /* 0x164692 */
    d0w = (int16_t)(d0w >> 2);                              /* 0x164696 asr.w #2 */
    d0w = (int16_t)(d0w - 8);                               /* 0x164698 */
    if (d0w < 0)                                            /* 0x16469A */
        d0w = (int16_t)(-d0w);                              /* 0x16469E */
    d0w = (int16_t)(d0w - 8);                               /* 0x1646A0 */
    d0w = (int16_t)(-d0w);                                  /* 0x1646A2 */
    uint16_t d1w = 4u;                                      /* 0x1646A4 */

    r->d[0] = setw(r->d[0], (uint16_t)d0w);
    r->d[1] = setw(r->d[1], d1w);

    /* 0x1646A8..0x164708: D0..D7/A0 saved & restored around line-RAM fill */
    {
        uint32_t lram = 0x00626200u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u); /* 0x1646B0..0x1646B6 */
        uint16_t b = d1w & 0x0Fu;                           /* 0x1646BA */
        if ((int16_t)b > 8)
            b = 8u;
        uint16_t a = (uint16_t)d0w & 0x0Fu;                 /* 0x1646CA */
        if ((int16_t)a > 8)
            a = 8u;
        uint16_t v1 = (uint16_t)(((b << 8) + (uint16_t)(8u - b)) << 4); /* 0x1646DA..0x1646E4 */
        uint16_t v0 = (uint16_t)((a << 8) + (uint16_t)(8u - a)); /* 0x1646E6..0x1646EE */
        uint16_t fill = (uint16_t)~(v1 | v0);               /* 0x1646F0..0x1646F2 */
        for (uint32_t i = 0; i < 0xE8u; i++)                /* 0x1646F4..0x164700 */
            wr16(m, lram + i * 2u, fill);
    }

    if (rd16(m, obj + 0x44u) == 0x0020u) {                  /* 0x16470C */
        wr8(m, F3_A5 + 0x4286u, 0u);                        /* 0x16471A..0x164722 (0x16AB58) */

        wr32(m, F3_A5 + 0x4234u, 0x00014000u);              /* 0x164728..0x16473C (0x16AB26) */
        wr32(m, F3_A5 + 0x4238u, 0u);

        uint32_t s0 = rd32(m, F3_A5 - 0x5D90u);             /* 0x16474A */
        uint32_t s1 = rd32(m, F3_A5 - 0x5D8Cu);             /* 0x16474E */
        wr32(m, F3_A5 + 0x4214u, s0);                       /* 0x164756 (0x16AB2C) */
        wr32(m, F3_A5 + 0x4218u, s1);
        wr32(m, F3_A5 - 0x5D98u, s0);
        wr32(m, F3_A5 - 0x5D94u, s1);

        r->d[0] = s0;
        r->d[1] = s1;
        r->a[1] = F3_A5 - 0x5D94u;

        Gfx_DrawTilemapAttr_00523A(m, 0x0006E0D4u, 0x00612000u, 0x00DEu, 0u); /* 0x164770 */
        Gfx_DrawTilemapAttr_00523A(m, 0x0006E0DCu, 0x00612400u, 0x00DEu, 0u); /* 0x16478E */
        Gfx_DrawTilemapAttr_00523A(m, 0x0006E0FEu, 0x00612020u, 0x00DEu, 0u); /* 0x1647AC */
        Gfx_DrawTilemapAttr_00523A(m, 0x0006E11Eu, 0x00612420u, 0x00DEu, 0u); /* 0x1647CA */
        Gfx_DrawTilemapAttr_00523A(m, 0x0006E164u, 0x00612040u, 0x00DEu, 0u); /* 0x1647E8 */
        Gfx_DrawTilemapAttr_00523A(m, 0x0006E190u, 0x00612440u, 0x00DEu, 0u); /* 0x164806 */
        Gfx_DrawTilemapAttr_00523A(m, 0x0006E1A6u, 0x00612060u, 0x00DEu, 0u); /* 0x164824 */
        Gfx_DrawTilemapAttr_00523A(m, 0x0006E1D2u, 0x00612460u, 0x00DEu, 0u); /* 0x164842 */

        Gfx_DrawCompressedTilemap(m, 0x00066D20u, 0x00610020u, 0u); /* 0x16485C */
        Gfx_DrawCompressedTilemap(m, 0x00066D20u, 0x00610420u, 0u); /* 0x164876 */
    }

    {
        int16_t cnt = (int16_t)(rd16(m, obj + 0x44u) - 1u); /* 0x164884 */
        wr16(m, obj + 0x44u, (uint16_t)cnt);
        if (cnt >= 0) {                                     /* 0x164888 */
            wr32(m, obj + 0x00u, 0x00164692u);              /* 0x16468A */
            return 0;
        }
    }

    Obj_Free(m, obj);                                       /* 0x16488C..0x164894 */
    return 0;
}
