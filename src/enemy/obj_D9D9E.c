#include "obj_D9D9E.h"
#include "../video/sprite.h"

/* ROM 0x0CEDE4 */
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
    wr32(m, obj + 0x30u, rd32(m, a1));   /* move.l (A1),(0x30,A0): A1 not advanced */
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u)); /* bset.b #1 */
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x20u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x18u));
    wr8(m, obj + 0x4Fu, 0u);
}

/* ROM 0x0D2D44: (D0, D1) = per-mode speed tables scaled by D2w */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    /* jump table 0x0D4F26 [0..50] -> 0x0D4FF2..0x0D538A: exact (D0*k, D1*k) */
    uint32_t d0 = rd32(m, 0x000D2D64u + (uint32_t)((int32_t)(int16_t)d0w * 4));
    uint32_t d1 = rd32(m, 0x000D2ECCu + (uint32_t)((int32_t)(int16_t)d0w * 4));
    *out_d0 = (uint32_t)(d0 * (uint32_t)d1w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d1w);
}

/* ROM 0x0D0678: animation script pointer + first frame */
static void sub_0D0678(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x48u, a1);
    wr16(m, obj + 0x4Cu, rd16(m, a1));
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1 + 2u));   /* move.b D0b: low byte */
    wr32(m, obj + 0x10u, rd32(m, a1 + 4u));
    wr32(m, obj + 0x68u, rd32(m, a1 + 8u));
}

/* ROM 0x0D9D9E */
int Obj_D9D9E_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t k = (uint16_t)(rd16(m, obj + 0x60u) + 5u);
    if (k > 50u)
        return -1;                          /* jump table 0x0D4F26 out of range */
    /* subi.l #0,(0x886,A5) is a no-op on memory */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 8u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u));
    sub_0CEDE4(m, obj, 0x000D9EB8u);        /* lea (0x108,PC) */
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));
    uint32_t d0, d1;
    sub_0D2D44(m, rd16(m, obj + 0x58u), k, &d0, &d1);
    wr32(m, obj + 0x50u, d0);
    wr32(m, obj + 0x54u, d1);
    sub_0D0678(m, obj, 0x000D9ED8u);        /* lea (0xf4,PC) */
    uint32_t sc = rd32(m, F3_A5 + 0x4228u);
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - sc);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - sc);
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x000D9E06u);
    return 0;
}
