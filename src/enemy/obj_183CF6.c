#include "obj_183CF6.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x0CC2BC: D0w = (A5+0x878) mod max(1, ((0x180 - A5+0x87E) * d0) >> 8) */
static uint16_t sub_0CC2BC(const f3mem_t *m, uint16_t d0w)
{
    uint16_t d1 = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t prod = (uint32_t)d0w * (uint32_t)d1;
    uint16_t mod = (uint16_t)((prod << 8) >> 16);
    if (mod == 0u)
        mod = 1u;
    return (uint16_t)(rd16(m, F3_A5 + 0x0878u) % mod);
}

/* ROM 0x183CF6: twin aimed projectile spawner */
int Obj_183CF6_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);
    if ((rd8(m, a4 + 0x23u) & 0x04u) == 0u) {
        if (sub_0CC2BC(m, 0x000Au) == 0u) {
            uint16_t x = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
            uint16_t y = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
            wr16(m, obj + 0x50u, x);
            wr16(m, obj + 0x54u, y);
            wr16(m, obj + 0x14u, x);
            wr16(m, obj + 0x18u, y);
            uint8_t ang = Obj_AimAtPlayer17AF9A(m, rd32(m, obj + 0x4Cu), x, y);
            uint32_t d1 = (uint32_t)ang;
            d1 = (d1 & 0xFFFFFF00u) | (uint8_t)(d1 - 5u);
            (void)Obj_Spawn(m, obj, 0x0018415Au, 9u, 2u, d1);
            d1 = (d1 & 0xFFFFFF00u) | (uint8_t)(d1 + 10u);
            (void)Obj_Spawn(m, obj, 0x0018415Au, 9u, 2u, d1);
        }
        int16_t cnt = (int16_t)(rd16(m, obj + 0x64u) - 1u);
        wr16(m, obj + 0x64u, (uint16_t)cnt);
        if (cnt > 0) {
            wr32(m, obj + 0x00u, 0x00183CF6u);
            return 0;
        }
    }
    wr16(m, obj + 0x06u, 1u);
    wr32(m, obj + 0x00u, 0x00183DA6u);
    return 0;
}
