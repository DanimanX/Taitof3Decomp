#include "obj_BD7E4.h"
#include "../objects/obj_pool.h"

#define ASR16_BD7E4(v) ((int16_t)(v) < 0 ? (uint16_t)(((uint16_t)(v) >> 1) | 0x8000u) : (uint16_t)((uint16_t)(v) >> 1))

/* ROM 0x176832: line-RAM zoom ramp. Only the Work RAM side effects are
 * reproduced (A5+0x4298 accumulator, A5+0x42A0); line RAM writes skipped. */
static void sub_176832(const f3mem_t *m, int16_t d7, int16_t d6, uint32_t acc,
                       uint32_t d5, uint32_t d3, uint16_t w42a0)
{
    wr32(m, F3_A5 + 0x4298u, acc);
    wr16(m, F3_A5 + 0x42A0u, w42a0);
    int16_t d4 = (d7 > d6) ? -1 : 1;
    for (;;) {
        d5 += d3;
        wr32(m, F3_A5 + 0x4298u, rd32(m, F3_A5 + 0x4298u) + d5);
        d7 = (int16_t)(d7 + d4);
        if (d4 < 0 ? !(d6 <= d7) : !(d7 <= d6))
            break;
    }
}

/* ROM 0x1768CA: line-RAM scroll ramp (Work RAM side effects only) */
static void sub_1768CA(const f3mem_t *m, int16_t d7, int16_t d6, uint32_t acc,
                       uint32_t d3, uint32_t d4)
{
    wr32(m, F3_A5 + 0x4298u, acc);
    if (rd16(m, F3_A5 - 0x5D58u) != 0u) {
        wr32(m, F3_A5 + 0x4298u, 0u - rd32(m, F3_A5 + 0x4298u));
        d3 = 0u - d3;
        d4 = 0u - d4;
    }
    int16_t d5 = (d7 > d6) ? -1 : 1;
    for (;;) {
        d3 += d4;
        wr32(m, F3_A5 + 0x4298u, rd32(m, F3_A5 + 0x4298u) + d3);
        d7 = (int16_t)(d7 + d5);
        if (d5 < 0 ? !(d6 <= d7) : !(d7 <= d6))
            break;
    }
}

/* ROM 0x0BD7E4 */
int Obj_BD7E4_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = ((uint32_t)rd16(m, obj + 0x60u) * 0x5000u) << 2;
    sub_176832(m, 0, 0xE7, d0, 0u, 0u, 3u);

    uint16_t d1 = (uint16_t)((uint16_t)ASR16_BD7E4(rd16(m, obj + 0x60u)) + 0x40u);
    d0 = 0x003FFFFFu;
    if (d1 == 0u)
        return -1; /* divide by zero trap */
    if (d0 / d1 <= 0xFFFFu)
        d0 = ((d0 % d1) << 16) | (d0 / d1);
    d0 &= 0xFFFFu;
    d0 = 0u - (d0 - 0xFFFFu);

    sub_1768CA(m, 0x74, 0, 0u, d0, 0u);
    sub_1768CA(m, 0x74, 0xE7, 0u, 0u - d0, 0u);

    uint16_t n = (uint16_t)(rd16(m, obj + 0x60u) + 1u);
    wr16(m, obj + 0x60u, n);
    if ((int16_t)n <= 0x58) {
        wr32(m, obj + 0x00u, 0x000BD7E4u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
