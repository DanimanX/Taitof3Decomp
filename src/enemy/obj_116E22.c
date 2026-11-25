#include "obj_116E22.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x116E22: tick for a 3D chain segment (spawned via 0x116DD2). Frees
 * when parent (+0x08) +0x50 bit 5 is set; otherwise forwards flash via
 * Obj_1170EE, registers at A5+0x30B6 when within +-0x1000 of front angle,
 * updates base 3D position via Obj_116BE6, projects offset via Obj_117154,
 * and selects sprite via Obj_117186. */
int Obj_116E22_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    if ((rd8(m, a1 + 0x50u) & 0x20u) != 0u) {
        Obj_Free(m, obj);
        return 0;
    }

    Obj_1170EE(m, obj, a1);

    int16_t ang = (int16_t)((uint16_t)rd32(m, obj + 0x0Cu) + rd16(m, F3_A5 + 0x30BAu) + 0x4000u);
    if (ang >= -0x1000 && ang <= 0x1000)
        wr32(m, F3_A5 + 0x30B6u, obj);

    wr16(m, obj + 0x20u, rd16(m, F3_A5 + 0x30BCu));
    wr16(m, obj + 0x54u, rd16(m, F3_A5 + 0x30BEu));
    Obj_116BE6(m, obj);

    uint16_t idx = (uint16_t)((uint16_t)rd32(m, obj + 0x0Cu) + rd16(m, F3_A5 + 0x30BAu)) >> 8;
    int32_t d1 = (int32_t)(int16_t)rd16(m, F3_A5 + 0x30BCu) * (int16_t)rd16(m, 0x00027E40u + (uint32_t)idx * 2u);
    d1 = (int32_t)((uint32_t)d1 << 2);
    uint16_t idx1 = (uint16_t)(((uint32_t)d1 >> 16) >> 8);
    int32_t d0 = (int32_t)(int16_t)0x00E0 * (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)idx1 * 2u);
    wr16(m, obj + 0x42u, (uint16_t)((uint32_t)d0 >> 16));

    int32_t d0_ang = (int32_t)(int16_t)rd16(m, obj + 0x20u) * (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)idx * 2u);
    d0_ang = (int32_t)((uint32_t)d0_ang << 2);
    uint16_t spr_ang = (uint16_t)(uint8_t)((((uint32_t)d0_ang >> 16) >> 8) + 0x74u);
    Obj_117154(m, obj, spr_ang);

    Obj_117186(m, obj, (uint16_t)rd32(m, obj + 0x0Cu), 0u);
    wr32(m, obj + 0x00u, 0x00116E22u);
    return 0;
}
