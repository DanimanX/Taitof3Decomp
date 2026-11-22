#include "obj_DA64C.h"

/* ROM 0x0D0CB0: global A5+0x87F >> 3 indexes table 0x0D0D82 */
static uint16_t sub_0D0CB0(const f3mem_t *m)
{
    uint16_t d0w = rd8(m, F3_A5 + 0x087Fu);
    d0w = (uint16_t)((uint8_t)d0w >> 3);
    return rd16(m, 0x000D0D82u + (uint32_t)d0w * 2u);
}

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

/* ROM 0x0CFA86: frame byte, sprite def, hit box */
static void sub_0CFA86(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    wr32(m, obj + 0x10u, rd32(m, a1 + 2u));
    wr32(m, obj + 0x68u, rd32(m, a1 + 6u));
}

/* ROM 0x0DA64C */
int Obj_DA64C_Tick(const f3mem_t *m, uint32_t obj)
{
    /* subi.l #0,(0x886,A5) is a no-op on memory */
    uint16_t k = (uint16_t)(sub_0D0CB0(m) + 0x14u);
    if (k > 50u)
        return -1;                          /* jump table 0x0D4F26 out of range */
    sub_0CEDE4(m, obj, 0x000DA804u);        /* lea (0x1ae,PC) */
    wr16(m, obj + 0x58u, rd16(m, obj + 0x10u));
    wr16(m, obj + 0x60u, (uint16_t)(rd16(m, obj + 0x36u) - 7u));
    wr8(m, obj + 0x1Fu, 0xF8u);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
    sub_0CFA86(m, obj, 0x000DA824u);        /* lea (0x1a8,PC) */
    uint32_t d0, d1;
    sub_0D2D44(m, 0x00B4u, k, &d0, &d1);
    wr32(m, obj + 0x50u, d0);
    wr32(m, obj + 0x54u, d1);
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
    wr32(m, obj + 0x00u, 0x000DA6C0u);
    return 0;
}
