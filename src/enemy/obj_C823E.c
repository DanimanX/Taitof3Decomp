#include "obj_C823E.h"
#include "../objects/obj_pool.h"

/* ROM 0x001660: queue a palette transfer {len, src, dst} (max 32 entries) */
static void pal_queue(const f3mem_t *m, uint32_t src, uint32_t dst, uint32_t len)
{
    uint16_t n = rd16(m, F3_A5 - 0x5DF0u);
    if ((int16_t)n >= 0x20)
        return;
    uint32_t e = F3_A5 - 0x6750u + (uint32_t)(int32_t)(int16_t)(uint16_t)(n << 3);
    wr16(m, e, (uint16_t)len);
    wr32(m, e + 2u, src);
    wr16(m, e + 6u, (uint16_t)dst);
    wr16(m, F3_A5 - 0x5DEEu, 2u);
    wr16(m, F3_A5 - 0x5DF0u, (uint16_t)(n + 1u));
}

/* ROM 0x0C839C */
static uint16_t lerp_c(uint16_t d5, uint16_t d6, uint16_t d0)
{
    d6 = (uint16_t)(d6 - d5);
    int32_t p = (int32_t)(int16_t)d6 * (int32_t)(int16_t)d0;
    int16_t q = (int16_t)(p / 100);
    return (uint16_t)((uint16_t)q + d5);
}

/* ROM 0x0C835A: blend 31 colours a1 -> a2 by d0/100 into a3 */
static void blend(const f3mem_t *m, uint32_t a1, uint32_t a2, uint32_t a3, uint16_t d0)
{
    for (int i = 0; i < 31; i++) {
        uint32_t d2 = rd32(m, a1), d3 = rd32(m, a2);
        a1 += 4u; a2 += 4u;
        uint32_t d4 = 0;
        for (int sh = 16; sh >= 0; sh -= 8) {
            uint16_t r = lerp_c((uint16_t)((d2 >> sh) & 0xFFu), (uint16_t)((d3 >> sh) & 0xFFu), d0);
            d4 |= (uint32_t)(r & 0xFFu) << sh;
        }
        wr32(m, a3, d4);
        a3 += 4u;
    }
}

/* ROM 0x0C823E: player palette fade (per player counter at +0x44/+0x46)
 * plus a cycling background palette; runs 0x15C frames */
int Obj_C823E_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = obj + 0x44u, a2 = 0x0041B190u, a3 = 0x0041B3F6u;
    for (int i = 0; i < 2; i++, a1 += 2u, a2 += 0x8Au, a3 += 0x7Cu) {
        if ((int16_t)rd16(m, a2 + 0x4Cu) < 0 || (rd8(m, a2 + 0x65u) & 0x04u) != 0u)
            continue;
        int16_t c = (int16_t)rd16(m, a1);
        if ((int16_t)rd16(m, obj + 0x06u) > 0xF8 || (int16_t)rd16(m, a2 + 0x4Eu) > 0) {
            if (c > 0) wr16(m, a1, (uint16_t)(c - 1));
        } else {
            if (c < 0x64) wr16(m, a1, (uint16_t)(c + 1));
        }
        uint32_t k = (uint32_t)(int32_t)(int16_t)rd16(m, a2 + 0x22u) * 4u;
        blend(m, rd32(m, 0x000C92DAu + k) + 4u, rd32(m, 0x000C92E2u + k), a3, rd16(m, a1));
        uint16_t d1 = (uint16_t)((uint16_t)(rd8(m, a2 + 0x0Cu) << 6) + 0x4004u);
        pal_queue(m, a3, d1, 0x1Fu);
    }
    uint16_t d0 = rd8(m, obj + 0x22u);
    wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) - 0x80u));
    int8_t b = (int8_t)rd8(m, obj + 0x22u);
    if (b < 0) {
        b = (int8_t)(b + 0x17);
        wr8(m, obj + 0x22u, (uint8_t)b);
    }
    if (b > 0x16)
        wr8(m, obj + 0x22u, (uint8_t)(b - 0x17));
    pal_queue(m, 0x000C83A8u + (uint32_t)d0 * 4u, 0x4504u, 0x17u);
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) + 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t <= 0x15C) {
        wr32(m, obj + 0x00u, 0x000C823Eu);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
