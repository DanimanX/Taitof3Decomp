#include "obj_C9A38.h"
#include "../objects/obj_pool.h"

/* ROM 0x0C9BCA: take a slot from the parent (+0x5A), place under it and
 * load hit box / motion records; on no slot give it back and free */
static void sub_0C9BCA(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    int16_t n = (int16_t)(rd16(m, a1 + 0x5Au) - 1u);
    wr16(m, a1 + 0x5Au, (uint16_t)n);
    if (n < 0 || (rd8(m, a1 + 0x65u) & 0x01u) != 0u) {
        a1 = rd32(m, obj + 0x08u);
        wr16(m, a1 + 0x5Au, (uint16_t)(rd16(m, a1 + 0x5Au) + 1u));
        Obj_Free(m, obj);
        return;
    }
    wr32(m, obj + 0x54u, 0u);
    wr32(m, obj + 0x50u, rd32(m, obj + 0x28u));
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0Fu));
    a1 = rd32(m, obj + 0x44u);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, a1))); a1 += 2u;
    wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x30u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x2Eu, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x32u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr32(m, obj + 0x44u, a1);
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    a1 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)(int16_t)(int8_t)rd8(m, a1))); a1++;
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)(int16_t)(int8_t)rd8(m, a1))); a1++;
    wr32(m, obj + 0x48u, (uint32_t)(int32_t)(int16_t)rd16(m, a1) << 8); a1 += 2u;
    wr32(m, obj + 0x4Cu, (uint32_t)(int32_t)(int16_t)rd16(m, a1) << 8);
    wr32(m, obj + 0x00u, 0x000C9C6Au);
}

/* ROM 0x0C9A38 */
int Obj_C9A38_Tick(const f3mem_t *m, uint32_t obj)
{
    wr8(m, obj + 0x22u, 1u);
    wr32(m, obj + 0x44u, 0x000C9A5Eu);
    wr32(m, obj + 0x24u, 0x00640000u);
    wr32(m, obj + 0x28u, 0x00009000u);
    wr16(m, obj + 0x1Eu, 0u);
    sub_0C9BCA(m, obj);
    return 0;
}
