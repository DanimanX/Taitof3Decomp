#include "obj_17AD18.h"
#include "../objects/obj_pool.h"

/* ROM 0x17B02E: D0 = score receiver for this object, or -1 */
static uint32_t sub_17B02E(const f3mem_t *m, uint32_t obj)
{
    uint8_t k = rd8(m, obj + 0x35u);
    if (k == 1u)
        return rd32(m, rd32(m, obj + 0x36u) + 0x08u);
    if (k == 2u)
        return rd32(m, obj + 0x36u);
    return 0xFFFFFFFFu;
}

/* ROM 0x17AD18: score popup. Takes the current score pointer from the spawn
 * argument, seeds the counter from the A5 globals and caches the receiver. */
int Obj_17AD18_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    wr32(m, obj + 0x44u, a1);
    wr16(m, obj + 0x48u, rd16(m, a1 + 0x04u));
    wr16(m, obj + 0x4Au, 0x0000u);
    uint32_t a4 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x50u, rd16(m, a4 + 0x50u));
    wr16(m, obj + 0x54u, rd16(m, a4 + 0x54u));
    wr16(m, obj + 0x34u, rd16(m, F3_A5 + 0x4B20u));
    wr32(m, obj + 0x36u, rd32(m, F3_A5 + 0x4B1Cu));
    wr32(m, obj + 0x4Cu, sub_17B02E(m, obj));
    wr32(m, obj + 0x00u, 0x17AD56u);
    return 0;
}
