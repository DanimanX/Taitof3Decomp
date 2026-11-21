#include "obj_C9DEC.h"
#include "../objects/obj_pool.h"

/* ROM 0x0C9E44: attach to the parent (+0x08) using the 4-byte record at +0x0C */
static void sub_0C9E44(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    a1 = rd32(m, obj + 0x0Cu);
    uint16_t d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1); a1++;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0w));
    d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1); a1++;
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0w));
    d0w = (uint16_t)((d0w & 0xFF00u) | rd8(m, a1)); a1++;
    wr8(m, obj + 0x4Du, rd8(m, a1));
    d0w &= 0x3u;
    uint32_t t = 0x000C9E86u + (uint32_t)d0w * 4u;
    wr16(m, obj + 0x20u, rd16(m, t));
    wr16(m, obj + 0x44u, rd16(m, t + 2u));
}

/* ROM 0x0C9E96: common option-shot init (shared with 0x0C9D7E family) */
static void sub_0C9E96(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    int16_t n = (int16_t)(rd16(m, a1 + 0x5Cu) - 1u);
    wr16(m, a1 + 0x5Cu, (uint16_t)n);
    if (n < 0 || (rd8(m, a1 + 0x65u) & 0x01u) != 0u) {
        /* 0x0CA00C */
        a1 = rd32(m, obj + 0x08u);
        wr16(m, a1 + 0x5Cu, (uint16_t)(rd16(m, a1 + 0x5Cu) + 1u));
        Obj_Free(m, obj);
        return;
    }
    wr8(m, obj + 0x22u, 0x04u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x2Cu, 0x0002u);
    wr16(m, obj + 0x2Eu, 0x000Du);
    wr16(m, obj + 0x30u, 0x0002u);
    wr16(m, obj + 0x32u, 0x000Du);
    wr32(m, obj + 0x24u, 0x00010000u);
    wr32(m, obj + 0x28u, 0x00018000u);
    uint32_t d0 = ((uint32_t)rd16(m, a1 + 0x56u) << 16) >> 6;
    wr32(m, obj + 0x28u, rd32(m, obj + 0x28u) + d0);
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    a1 = 0x000CA910u;
    if (rd8(m, obj + 0x4Eu) != 0u)
        a1 += 0x40u;
    else if (rd8(m, obj + 0x4Du) != 0u)
        a1 += 0x20u;
    wr32(m, obj + 0x48u, a1);
    wr16(m, obj + 0x46u, 0x0000u);
    wr32(m, obj + 0x00u, 0x000C9F2Eu);
}

/* ROM 0x0C9DEC */
int Obj_C9DEC_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0C9E44(m, obj);
    uint32_t a1 = rd32(m, rd32(m, obj + 0x08u) + 0x44u);
    uint16_t d1w = 0u, d0w = 0u;
    if ((rd8(m, a1) & 0x08u) == 0u) {
        d1w = (uint16_t)-0x1000;
        d0w = (uint16_t)((int16_t)rd16(m, obj + 0x44u) >> 1);
    }
    if ((rd8(m, a1) & 0x04u) == 0u) {
        d1w = 0x0C00u;
        d0w = rd16(m, obj + 0x44u);
    }
    d0w = (uint16_t)((int16_t)d0w >> 2);
    if ((int16_t)rd16(m, obj + 0x20u) < 0)
        d1w = (uint16_t)(0u - d1w);
    wr16(m, obj + 0x44u, (uint16_t)(rd16(m, obj + 0x44u) + d0w));
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + d1w));
    wr16(m, obj + 0x42u, 0x0008u);
    wr8(m, obj + 0x4Cu, 0x00u);   /* sf */
    wr8(m, obj + 0x4Eu, 0xFFu);   /* st */
    sub_0C9E96(m, obj);
    return 0;
}
