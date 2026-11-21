#include "obj_11C3A8.h"
#include "../objects/obj_pool.h"

#define ASR32(v, n) ((int32_t)(((int32_t)(v) < 0) \
    ? ~(uint32_t)((uint32_t)~(uint32_t)(v) >> (n)) \
    : ((uint32_t)(v) >> (n))))

/* ROM 0x001660: queue a palette upload {len.w, src.l, dst.w} at A5-0x6750 */
static void sub_001660(const f3mem_t *m, uint32_t a0, uint32_t d0, uint32_t d1)
{
    uint16_t n = rd16(m, F3_A5 - 0x5DF0u);
    if ((int16_t)n >= 0x20)
        return;
    uint32_t a2 = F3_A5 - 0x6750u + (uint32_t)(int32_t)(int16_t)(uint16_t)(n << 3);
    wr16(m, a2, (uint16_t)d1);
    wr32(m, a2 + 2u, a0);
    wr16(m, a2 + 6u, (uint16_t)d0);
    wr16(m, F3_A5 - 0x5DEEu, 2u);
    wr16(m, F3_A5 - 0x5DF0u, (uint16_t)(n + 1u));
}

/* ROM 0x11C458: D6w += (D5w - D6w) * D0w >> 8 */
static uint16_t sub_11C458(uint16_t d0w, uint16_t d5w, uint16_t d6w)
{
    d5w = (uint16_t)(d5w - d6w);
    int32_t d5 = (int32_t)(int16_t)d5w * (int32_t)(int16_t)d0w;
    d5 = ASR32(d5, 8u);
    return (uint16_t)(d6w + (uint16_t)d5);
}

/* ROM 0x11C3A8: palette blend between 0x11C462 and 0x11C4E2 */
int Obj_11C3A8_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 - 0x144Bu) & 0x10u) {
        sub_001660(m, 0x0011C4E2u, 0x4400u, 0x20u);
        Obj_Free(m, obj);
        return 0;
    }

    int16_t div = (int16_t)rd16(m, obj + 0x24u);
    if (div == 0)
        return -1; /* divide-by-zero trap */
    uint32_t d0 = (uint32_t)ASR32(rd32(m, F3_A5 + 0x30DAu), 8u);
    {
        int32_t q = (int32_t)d0 / (int32_t)div;
        int32_t r = (int32_t)d0 % (int32_t)div;
        if (q >= -32768 && q <= 32767)
            d0 = ((uint32_t)(uint16_t)r << 16) | (uint16_t)q;
    }
    uint16_t d0w = (uint16_t)d0;

    uint32_t a1 = 0x0011C462u, a2 = 0x0011C4E2u, a3 = 0x0040B11Eu;
    for (int i = 0; i < 32; i++, a1 += 4u, a2 += 4u, a3 += 4u) {
        uint32_t d2 = rd32(m, a1), d3 = rd32(m, a2), d4 = 0u;
        static const unsigned sh[3] = { 16u, 8u, 0u };
        for (int k = 0; k < 3; k++) {
            uint16_t d6 = sub_11C458(d0w, (uint16_t)((d2 >> sh[k]) & 0xFFu),
                                     (uint16_t)((d3 >> sh[k]) & 0xFFu));
            d4 |= ((uint32_t)d6 & 0xFFu) << sh[k];
        }
        wr32(m, a3, d4);
    }
    sub_001660(m, 0x0040B11Eu, 0x4400u, 0x20u);
    wr32(m, obj + 0x00u, 0x0011C3A8u);
    return 0;
}
