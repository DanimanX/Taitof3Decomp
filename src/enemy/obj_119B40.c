#include "obj_119B40.h"
#include "../objects/obj_common.h"

/* ROM 0x119B40: debris init; +0xC = x:10s | y:10s | angle:8 | 4 */
int Obj_119B40_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x14u, (uint16_t)(int16_t)((int32_t)d0 >> 22));
    wr16(m, obj + 0x18u, (uint16_t)(int16_t)((int32_t)(d0 << 10) >> 22));
    wr16(m, obj + 0x20u, (uint16_t)((d0 >> 4) & 0xFFu));

    uint16_t r = (uint16_t)(Rng_NextMem(m) & 3u);
    uint32_t a1 = 0x00119C1Au + (uint32_t)r * 8u;   /* lea (0xac,PC) */
    wr16(m, obj + 0x42u, rd16(m, a1));
    wr16(m, obj + 0x3Au, rd16(m, a1 + 2u));
    wr32(m, obj + 0x0Cu, rd32(m, a1 + 4u));
    r = (uint16_t)(Rng_NextMem(m) & 3u);
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + r));

    uint32_t ang = rd16(m, obj + 0x20u);
    int16_t sp = (int16_t)rd16(m, obj + 0x42u);
    int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + ang * 2u);
    int16_t s = (int16_t)rd16(m, 0x00027E40u + ang * 2u);
    wr32(m, obj + 0x2Cu, rd32(m, obj + 0x2Cu) + (uint32_t)((int32_t)sp * c));
    wr32(m, obj + 0x30u, rd32(m, obj + 0x30u) + (uint32_t)((int32_t)sp * s));
    wr32(m, obj + 0x00u, 0x00119BBEu);
    return 0;
}
