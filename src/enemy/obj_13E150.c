#include "obj_13E150.h"
#include "../objects/obj_pool.h"

/* ROM 0x00523A (defined in obj_14C8F6.c) */
void Gfx_DrawTilemapAttr_00523A(const f3mem_t *m, uint32_t src, uint32_t dst,
                                uint16_t attr, uint16_t flags);

/* ROM 0x13E8E0: set the line-ram scroll base word (A5+0x3300) and OR 4 into
 * all 0xE7 words of 0x620800 (+2 when the side flag A5-0x5D58 is clear). */
static void sub_13E8E0(const f3mem_t *m)
{
    uint32_t a0 = 0x00620800u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    if (rd8(m, F3_A5 - 0x5D58u) != 0u) {
        wr16(m, F3_A5 + 0x3300u, 0x01CEu);
    } else {
        wr16(m, F3_A5 + 0x3300u, 0x0030u);
        a0 += 2u;
    }
    uint16_t d1 = (uint16_t)(rd16(m, a0) | 0x0004u);
    for (int i = 0; i <= 0xE6; i++) {
        wr16(m, a0, d1);
        a0 += 2u;
    }
}

/* ROM 0x13E3C4: build the two 0x1D0-byte zoom tables (0x628400 / 0x62A400)
 * and the 0x6244xx scroll-window table. */
static void sub_13E3C4(const f3mem_t *m)
{
    sub_13E8E0(m);
    uint32_t adj = (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    uint32_t a1 = 0x00628400u + adj;
    uint32_t a2 = 0x0062A400u + adj;
    uint32_t a3 = a1 + 0x1D0u;
    uint32_t a4 = a2 + 0x1D0u;
    uint32_t d1 = 0x00100000u;
    for (int n = 0; n <= 0x73; n++) {
        uint16_t d5w = (uint16_t)(((d1 << 8) >> 16) & 0xFFFFu);   /* lsl.l #8; swap */
        uint16_t q = (uint16_t)(0x10000u / d5w);                  /* divu.w */
        uint16_t d3w = (uint16_t)((uint16_t)((uint16_t)(0x100u - q) << 8) | 0x80u);
        d3w = (uint16_t)((d3w & 0xFF00u) | 0x80u);
        wr16(m, a1, d3w); a1 += 2u;
        a3 -= 2u; wr16(m, a3, d3w);

        int32_t d4 = (int32_t)(0x100u - (uint32_t)(d3w >> 8));
        int32_t dd = (int32_t)(0x01000000 / d4);                  /* divsl.l */
        int32_t t = (int32_t)((uint32_t)(-0x00B00000) << 6);      /* neg.l; asl.l #6 */
        int32_t r = t / dd;                                       /* divsl.l */
        uint16_t v = (uint16_t)(((uint16_t)r + 0x0400u) ^ 0x3Fu);
        wr16(m, a2, v); a2 += 2u;                                 /* (A2)+ */
        a4 -= 2u; wr16(m, a4, v);                                 /* -(A4): same value (D3 restored) */
        d1 -= 0x2164u;
    }

    uint32_t b1 = 0x006244E8u + adj;
    uint32_t b2 = b1;
    int flag = rd8(m, F3_A5 - 0x5D58u) != 0u;
    uint32_t d2 = 0x00020000u;
    d1 = 0;
    for (int n = 0x73; n >= 0; n--) {
        uint16_t x = (uint16_t)(d1 >> 16);
        uint16_t d3 = (uint16_t)((x - (uint16_t)n) & 0x1FFu);
        uint16_t d4 = (uint16_t)(x + (uint16_t)n);
        if (flag) { uint16_t tmp = d3; d3 = d4; d4 = tmp; }
        d4 &= 0x1FFu;
        b1 -= 2u; wr16(m, b1, d3);
        wr16(m, b2, d4); b2 += 2u;
        d1 = flag ? d1 + d2 : d1 - d2;
        d2 -= (uint32_t)((int32_t)d2 >> 6);
    }
}

/* ROM 0x13E150: init. Unpacks the 0x6E164 tilemap (attr 0x28) into 16
 * pf blocks, builds the zoom/scroll tables, seeds +0x14/+0x18 and installs
 * tick 0x13E34C. */
int Obj_13E150_Tick(const f3mem_t *m, uint32_t obj)
{
    static const uint16_t blk[4] = { 0x2000u, 0x2400u, 0x2800u, 0x2C00u };
    for (int b = 0; b < 4; b++)
        for (uint32_t i = 0; i < 4u; i++)
            Gfx_DrawTilemapAttr_00523A(m, 0x0006E164u, 0x00610000u + blk[b] + i * 0x20u, 0x0028u, 0x0200u);
    sub_13E3C4(m);
    wr32(m, obj + 0x14u, 0xFFFE0000u);
    wr32(m, obj + 0x18u, 0x00040000u);
    wr32(m, obj + 0x00u, 0x0013E34Cu);
    return 0;
}
