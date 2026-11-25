#include "obj_116C9A.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x116C9A: tick for a trailing 3D chain sub-segment (spawned by 0x116B0C
 * via 0x116C48). Frees when grandparent (parent->+0x08) +0x50 bit 5 is set;
 * otherwise forwards damage/flash to grandparent, eases +0x20/+0x54 toward
 * target (+0x0C), projects offset via Obj_117154, selects sprite via
 * Obj_117186, and periodically spawns 0x1177AC shots when tail (+0x50 == 0). */
int Obj_116C9A_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t parent = rd32(m, obj + 0x08u);
    uint32_t grand = rd32(m, parent + 0x08u);
    if ((rd8(m, grand + 0x50u) & 0x20u) != 0u) {
        Obj_Free(m, obj);
        return 0;
    }

    Obj_117058(m, obj, grand);

    uint32_t target = rd32(m, obj + 0x0Cu);
    int16_t diff20 = (int16_t)(rd16(m, target + 0x20u) - rd16(m, obj + 0x20u));
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + (diff20 >> 3)));
    int16_t diff54 = (int16_t)(rd16(m, target + 0x54u) - rd16(m, obj + 0x54u));
    wr16(m, obj + 0x54u, (uint16_t)(rd16(m, obj + 0x54u) + (diff54 >> 3)));

    uint16_t idx = (uint16_t)((uint16_t)rd32(m, parent + 0x0Cu) + rd16(m, F3_A5 + 0x30BAu)) >> 8;
    int32_t d1 = (int32_t)(int16_t)rd16(m, F3_A5 + 0x30BCu) * (int16_t)rd16(m, 0x00027E40u + (uint32_t)idx * 2u);
    d1 = (int32_t)((uint32_t)d1 << 2);
    uint16_t idx1 = (uint16_t)(((uint32_t)d1 >> 16) >> 8);
    int32_t d0 = (int32_t)(int16_t)0x00F0 * (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)idx1 * 2u);
    wr16(m, obj + 0x42u, (uint16_t)((uint32_t)d0 >> 16));

    wr32(m, obj + 0x14u, rd32(m, target + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, target + 0x18u));

    int32_t d0_ang = (int32_t)(int16_t)rd16(m, obj + 0x20u) * (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)idx * 2u);
    d0_ang = (int32_t)((uint32_t)d0_ang << 2);
    uint16_t ang = (uint16_t)(uint8_t)((((uint32_t)d0_ang >> 16) >> 8) + 0x74u);
    wr16(m, obj + 0x06u, ang);
    Obj_117154(m, obj, ang);

    Obj_117186(m, obj, (uint16_t)rd32(m, parent + 0x0Cu), rd16(m, obj + 0x22u));

    if (rd32(m, obj + 0x50u) == 0u) {
        uint8_t flg51 = rd8(m, grand + 0x51u);
        uint8_t flg50 = rd8(m, grand + 0x50u);
        if ((flg51 & 0x01u) != 0u && ((flg50 & 0x08u) != 0u || (flg50 & 0x01u) == 0u)) {
            if ((rd16(m, F3_A5 + 0x0878u) & 0x000Fu) == 0u) {
                uint32_t arg = (uint32_t)(uint16_t)(rd16(m, obj + 0x06u) << 8);
                if ((flg50 & 0x08u) != 0u)
                    arg |= 0x000000FFu;
                (void)Obj_Spawn(m, obj, 0x001177ACu, 9u, 2u, arg);
            }
        }
    }

    wr32(m, obj + 0x00u, 0x00116C9Au);
    return 0;
}
