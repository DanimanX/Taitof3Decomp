#include "obj_112D1A.h"
#include "obj_112C1A.h"

/* ROM 0x001660: queue a palette upload {count.w, src.l, dst.w} (max 0x20) */
static void sub_001660(const f3mem_t *m, uint32_t src, uint16_t dst, uint16_t count)
{
    uint16_t n = rd16(m, F3_A5 - 0x5DF0u);
    if ((int16_t)n >= 0x20)
        return;
    uint32_t a2 = F3_A5 - 0x6750u + (uint32_t)(int32_t)(int16_t)(uint16_t)(n << 3);
    wr16(m, a2, count);
    wr32(m, a2 + 2u, src);
    wr16(m, a2 + 6u, dst);
    wr16(m, F3_A5 - 0x5DEEu, 0x0002u);
    wr16(m, F3_A5 - 0x5DF0u, (uint16_t)(n + 1u));
}

/* ROM 0x112D1A */
int Obj_112D1A_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_1133AE_LoadPart(m, obj);
    wr32(m, obj + 0x28u, 0x00010000u);
    wr8(m, obj + 0x4Fu, 0u);
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x24u, rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x0Cu, a1);
    uint32_t src = rd32(m, a1);
    uint16_t dst = rd16(m, a1 + 8u);        /* move.l then move.w into D1w */
    sub_001660(m, src, dst, 0x20u);

    /* 0x112D50 */
    uint16_t d0 = (uint16_t)(rd16(m, F3_A5 + 0x087Eu) << 1);
    wr16(m, obj + 0x24u, (uint16_t)(rd16(m, obj + 0x24u) + d0));
    if ((int16_t)rd16(m, obj + 0x24u) < 0x14)
        wr16(m, obj + 0x24u, 0x0014u);
    if ((int16_t)rd16(m, obj + 0x24u) > 0xC8)
        wr16(m, obj + 0x24u, 0x00C8u);
    wr32(m, obj + 0x56u, rd32(m, obj + 0x24u));
    wr32(m, obj + 0x00u, 0x00112D88u);   /* 0x112D80 */
    return 0;
}
