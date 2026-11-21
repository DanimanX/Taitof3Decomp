#include "obj_102F52.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x0CEDE4: object template init (params at A1, 0x14 header fields) */
static void sub_0CEDE4(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    a1 += 0x14u;

    wr32(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x24u, rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x28u, rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1));
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u)); /* bset.b #1 */
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);
}

/* ROM 0x0CFA86: sprite def {colour.b, x.b, y.b, tile.w, attr.w} into the obj */
static void sub_0CFA86(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)d0);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D0CB0: global A5+0x87F >> 3 indexes table 0x0D0D82 */
static uint16_t sub_0D0CB0(const f3mem_t *m)
{
    uint16_t d0w = rd8(m, F3_A5 + 0x087Fu);
    d0w = (uint16_t)((uint8_t)d0w >> 3);
    return rd16(m, 0x000D0D82u + (uint32_t)d0w * 2u);
}

/* ROM 0x0D2D44: (D0, D1) = per-mode speed tables scaled by D2w */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    /* jump table 0x0D4F26 [0..50] -> 0x0D4FF2..0x0D538A: exact (D0*k, D1*k) */
    uint32_t d0 = rd32(m, 0x000D2D64u + (uint32_t)(int32_t)((int16_t)d0w * 4));
    uint32_t d1 = rd32(m, 0x000D2ECCu + (uint32_t)(int32_t)((int16_t)d0w * 4));
    *out_d0 = (uint32_t)(d0 * (uint32_t)d1w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d1w);
}

/* ROM 0x102F52 */
int Obj_102F52_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x102F52: lea (0x1BC,PC),A1 */
    sub_0CEDE4(m, obj, (uint32_t)(0x102F54u + 0x1BCu));

    /* 0x102F5C */
    wr8(m, obj + 0x21u, 0x04u);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));  /* bset.b #4 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u)); /* bclr.b #1 */

    /* 0x102F6E: D0w = variant */
    uint16_t d0w = rd16(m, obj + 0x10u);
    wr16(m, obj + 0x58u, rd16(m, 0x001030AEu + (uint32_t)d0w * 2u));
    wr32(m, obj + 0x0Cu, rd32(m, 0x001030BCu + (uint32_t)d0w * 4u));

    uint32_t a1 = 0x001030D8u + (uint32_t)(int32_t)((int16_t)d0w * 8);
    wr16(m, obj + 0x2Cu, rd16(m, a1));
    a1 += 2u;
    wr16(m, obj + 0x2Eu, rd16(m, a1));
    a1 += 2u;
    wr16(m, obj + 0x30u, rd16(m, a1));
    a1 += 2u;
    wr16(m, obj + 0x32u, rd16(m, a1));

    /* 0x102F9E: movea.l (-0x18E,PC,D0w*4),A1 (sign-extended 16-bit offset) */
    uint32_t a1b = rd32(m, 0x00102E12u + (uint32_t)(int32_t)((int16_t)d0w * 4));

    /* 0x102FA4 */
    sub_0CFA86(m, obj, a1b);

    uint16_t t = (uint16_t)(sub_0D0CB0(m) + 0x0Du);
    wr16(m, obj + 0x60u, t);
    uint32_t out_d0, out_d1;
    sub_0D2D44(m, rd16(m, obj + 0x58u), t, &out_d0, &out_d1);
    wr32(m, obj + 0x50u, out_d0);
    wr32(m, obj + 0x54u, out_d1);

    uint32_t d0 = rd32(m, F3_A5 + 0x4228u);
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - d0);

    Obj_DrawSprite(m, obj);

    /* 0x102FDE */
    wr32(m, obj + 0x00u, 0x00102FE6u);
    return 0;
}
