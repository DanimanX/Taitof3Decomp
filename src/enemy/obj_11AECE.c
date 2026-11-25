#include "obj_11AECE.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x11AECE: pull/push scroll +0x30D2/+0x30D6 while parent alive, else free */
int Obj_11AECE_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    if (rd8(m, a1 + 0x59u) != 0u) {
        Obj_Free(m, obj);
        return 0;
    }
    if (rd8(m, F3_A5 + 0x30F3u) == 0u &&
        (rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        if ((Rng_NextMem(m) & 1u) == 0u) {
            int16_t spd = (int16_t)((Rng_NextMem(m) & 0x000Fu) - 2u);
            wr16(m, obj + 0x42u, (uint16_t)spd);
            int16_t dx = (int16_t)(rd16(m, F3_A5 + 0x30D2u) - 0x00C8u);
            int16_t dy = (int16_t)(rd16(m, F3_A5 + 0x30D6u) - 0x0074u);
            uint8_t ang = (uint8_t)(Obj_AtanByte(m, dx, dy) + 0x78u);
            int32_t vx = (int32_t)spd * (int32_t)(int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)ang * 2u);
            int32_t vy = (int32_t)spd * (int32_t)(int16_t)rd16(m, 0x00027E40u + (uint32_t)ang * 2u);
            wr32(m, obj + 0x14u, (uint32_t)((int32_t)rd32(m, obj + 0x14u) + vx));
            wr32(m, F3_A5 + 0x30D2u, (uint32_t)((int32_t)rd32(m, F3_A5 + 0x30D2u) + vx));
            wr32(m, obj + 0x18u, (uint32_t)((int32_t)rd32(m, obj + 0x18u) + vy));
            wr32(m, F3_A5 + 0x30D6u, (uint32_t)((int32_t)rd32(m, F3_A5 + 0x30D6u) + vy));
        }
        wr32(m, obj + 0x00u, 0x0011AECEu);
        return 0;
    }
    int32_t d0 = (int32_t)rd32(m, obj + 0x14u);
    if (d0 >= 0x20 || d0 < -0x20)
        d0 >>= 5;
    wr32(m, obj + 0x14u, (uint32_t)((int32_t)rd32(m, obj + 0x14u) - d0));
    wr32(m, F3_A5 + 0x30D2u, (uint32_t)((int32_t)rd32(m, F3_A5 + 0x30D2u) - d0));
    d0 = (int32_t)rd32(m, obj + 0x18u);
    if (d0 >= 0x20 || d0 < -0x20)
        d0 >>= 5;
    wr32(m, obj + 0x18u, (uint32_t)((int32_t)rd32(m, obj + 0x18u) - d0));
    wr32(m, F3_A5 + 0x30D6u, (uint32_t)((int32_t)rd32(m, F3_A5 + 0x30D6u) - d0));
    wr32(m, obj + 0x00u, 0x0011AECEu);
    return 0;
}
