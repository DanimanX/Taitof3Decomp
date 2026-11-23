#include "obj_D7DD8.h"
#include "../objects/obj_common.h"

/* ROM 0x0CBE66: hit/collision record at A1 */
static void sub_0CBE66(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr16(m, obj + 0x4Cu, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x24u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x28u, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)(int8_t)rd8(m, a1)); a1++;
    wr16(m, obj + 0x30u, (uint16_t)(int16_t)(int8_t)rd8(m, a1)); a1++;
    wr16(m, obj + 0x2Eu, (uint16_t)(int16_t)(int8_t)rd8(m, a1)); a1++;
    wr16(m, obj + 0x32u, (uint16_t)(int16_t)(int8_t)rd8(m, a1)); a1++;
    wr8(m, obj + 0x34u, rd8(m, a1)); a1++;
    wr8(m, obj + 0x65u, rd8(m, a1));
    wr16(m, obj + 0x22u, rd16(m, obj + 0x0Eu));
    wr8(m, obj + 0x4Eu, rd8(m, obj + 0x0Cu));
}


/* ROM 0x0D7DD8 */
int Obj_D7DD8_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0;
    Obj_ResetFields(m, obj);
    if (rd8(m, obj + 0x4Eu) != 0u) {
        uint32_t a1 = rd32(m, F3_A5 + 0x1D28u);
        wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x18u));
        wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x10u));
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0xA0u));
        d0 = rd32(m, obj + 0x14u);
        if ((int16_t)rd16(m, obj + 0x14u) <= -0x50) {
            wr8(m, obj + 0x64u, 0u);
            d0 = d0 + 0x500000u - 0x2800000u;
        } else {
            wr8(m, obj + 0x64u, 0xFFu);
            d0 = d0 - 0x500000u + 0x2800000u;
        }
    } else {
        d0 = rd32(m, obj + 0x14u) - 0xA00000u;
        if ((int32_t)d0 < 0) {
            wr8(m, obj + 0x64u, 0xFFu);
            d0 += 0x2800000u;
        } else {
            wr8(m, obj + 0x64u, 0u);
            d0 -= 0x2800000u;
        }
    }
    wr32(m, obj + 0x54u, d0);
    wr32(m, obj + 0x58u, rd32(m, obj + 0x18u) - 0x740000u);
    sub_0CBE66(m, obj, 0xD8530u);
    wr8(m, obj + 0x22u, 0u);
    wr16(m, obj + 0x42u, 0u);
    wr16(m, obj + 0x20u, 0xC0u);
    wr16(m, obj + 0x5Cu, 0u);
    wr16(m, obj + 0x60u, 0x80u);
    wr16(m, obj + 0x06u, 0xA9u);
    wr16(m, obj + 0x5Eu, 4u);
    wr32(m, obj, 0xD7EAEu);
    return 0;
}
