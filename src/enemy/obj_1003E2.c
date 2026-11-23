#include "obj_1003E2.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../audio/sound_cmd.h"

/* ROM 0x0CEC4C: template init (0x0CEDE4 body) plus the per-group work
 * block at obj+0x0C (jump table 0x0CECE4) */
static void sub_0CEC4C(const f3mem_t *m, uint32_t obj, uint32_t a1)
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
    a1 += 4u;
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u)); /* bset.b #1 */
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);

    /* D0.w = obj+0x05 (+0x20 unless group == 5); jump table 0x0CECE4 */
    uint16_t d0 = rd8(m, obj + 0x05u);
    if (rd8(m, obj + 0x04u) != 5u)
        d0 = (uint16_t)(d0 + 0x20u);
    uint32_t a2 = rd32(m, 0x000CECE4u + (uint32_t)d0 * 4u);

    wr32(m, obj + 0x0Cu, a2);
    wr32(m, a2 + 0x08u, 0u);
    wr16(m, a2 + 0x08u, rd16(m, a1));
    a1 += 2u;
    wr32(m, a2 + 0x0Cu, rd32(m, a1));
    a1 += 4u;
    wr32(m, a2 + 0x10u, 0u);
    wr16(m, a2 + 0x10u, rd16(m, a1));
    a1 += 2u;
    wr32(m, a2 + 0x14u, rd32(m, a1));
    a1 += 4u;
    wr32(m, a2 + 0x18u, 0u);
    wr16(m, a2 + 0x18u, rd16(m, a1));
    a1 += 2u;
    wr32(m, a2 + 0x1Cu, rd32(m, a1));
    a1 += 4u;
    wr32(m, a2 + 0x20u, 0u);
    wr16(m, a2 + 0x20u, rd16(m, a1));
    a1 += 2u;
    wr32(m, a2 + 0x24u, rd32(m, a1));   /* move.l (A1): not consumed */
}
/* ROM 0x0CFA86: sprite def {frame.w, tile/attr.l, +0x68 ptr} */
static void sub_0CFA86(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)d0);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}
/* ROM 0x0D0E3A: LCG step at A5+0x1D60 (n = n*5+1); returns n >> 8 */

/* ROM 0x0D2D44: (D0, D1) = per-angle speed tables scaled by D1w (0..50) */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    /* jump table 0x0D4F26 [0..50] -> exact (D0*k, D1*k) */
    *out_d0 = (uint32_t)(d0 * (uint32_t)d1w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d1w);
}

/* ROM 0x1003E2 */
int Obj_1003E2_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x1003E2: A3 = 0x1005C2 + D0w*8, two longwords {template, sprite def} */
    uint16_t d0 = rd16(m, obj + 0x10u);
    uint32_t a3 = 0x001005C2u + (uint32_t)(int32_t)(int16_t)d0 * 8u;

    sub_0CEC4C(m, obj, rd32(m, a3));
    a3 += 4u;
    sub_0CFA86(m, obj, rd32(m, a3));

    /* 0x1003FE */
    wr8(m, obj + 0x1Du, 0xCAu);
    wr8(m, obj + 0x4Fu, 0xCAu);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));  /* bset.b #4 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & (uint8_t)~0x02u)); /* bclr.b #1 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));  /* bset.b #2 */

    /* 0x10041C */
    d0 = (uint16_t)(Obj_Next4Way(m) & 0x3u);
    wr16(m, obj + 0x60u, (uint16_t)(rd16(m, obj + 0x60u) + d0));

    d0 = (uint16_t)((Obj_Next4Way(m) & 0x3Fu) - 0x20u);
    d0 = (uint16_t)(rd16(m, obj + 0x58u) + d0);
    if ((int16_t)d0 < 0) {
        d0 = (uint16_t)(d0 + 0x168u);
    } else if (d0 >= 0x168u) {
        d0 = (uint16_t)(d0 - 0x168u);
    }

    /* 0x100454 */
    uint32_t out0, out1;
    sub_0D2D44(m, d0, rd16(m, obj + 0x60u), &out0, &out1);
    wr32(m, obj + 0x50u, out0);
    wr32(m, obj + 0x54u, out1);

    /* 0x100466 */
    uint32_t s = rd32(m, F3_A5 + 0x4224u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - s);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - s);
    s = rd32(m, F3_A5 + 0x4228u);
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - s);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - s);

    /* 0x10047E */
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x0010048Cu);
    return 0;
}
