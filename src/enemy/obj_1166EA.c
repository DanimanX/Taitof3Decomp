#include "obj_1166EA.h"
#include "../objects/obj_pool.h"

/* ROM 0x1166EA: countdown +0x06, unlink, spawn boss parts, arm 0x116802 */
int Obj_1166EA_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x001166EAu);
        return 0;
    }
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint32_t a2 = rd32(m, obj + 0x48u);
    wr32(m, a2 + 0x48u, a1);
    wr32(m, a1 + 0x44u, a2);
    (void)Obj_Spawn(m, obj, 0x00116EDCu, 3u, 2u, 0u);
    (void)Obj_Spawn(m, obj, 0x00112D1Au, 3u, 2u, 0x001175DEu);
    (void)Obj_Spawn(m, obj, 0x00112C1Au, 3u, 2u, 0x001176AAu);
    (void)Obj_Spawn(m, obj, 0x00112C1Au, 3u, 2u, 0x00117696u);
    (void)Obj_Spawn(m, obj, 0x00112C1Au, 3u, 2u, 0x001176A0u);
    (void)Obj_Spawn(m, obj, 0x00116F7Au, 3u, 2u, 0u);
    (void)Obj_Spawn(m, obj, 0x001134B2u, 3u, 2u, 0u);
    wr8(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x35u, 0u);
    wr8(m, obj + 0x1Du, 0u);
    wr8(m, obj + 0x4Fu, 0u);
    wr16(m, F3_A5 + 0x30BCu, 0x3A00u);
    wr16(m, obj + 0x3Eu, 0u);
    wr16(m, obj + 0x40u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr16(m, obj + 0x06u, 0x00B4u);
    wr32(m, obj + 0x3Au, rd32(m, F3_A5 + 0x1D28u));
    wr32(m, obj + 0x00u, 0x00116802u);
    return 0;
}
