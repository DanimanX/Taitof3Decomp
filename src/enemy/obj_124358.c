#include "obj_124358.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x124358: spawn 0x13E4EE / 0x13EE08 effects, count down +0x06, cleanup via 0x11AE2C */
int Obj_124358_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, F3_A5 + 0x32FAu, rd32(m, F3_A5 + 0x30D2u));
    wr16(m, F3_A5 + 0x32FAu, (uint16_t)(rd16(m, F3_A5 + 0x32FAu) - 0x0022u));
    wr32(m, F3_A5 + 0x32FCu, rd32(m, F3_A5 + 0x30D6u));
    wr16(m, F3_A5 + 0x32FCu, (uint16_t)(rd16(m, F3_A5 + 0x32FCu) - 0x0006u));
    wr16(m, obj + 0x14u, rd16(m, F3_A5 + 0x30D2u));
    wr16(m, obj + 0x18u, rd16(m, F3_A5 + 0x30D6u));
    if ((rd16(m, obj + 0x06u) & 0x0003u) == 0u) {
        uint8_t b0 = (uint8_t)((Rng_NextMem(m) & 0x007Fu) - 0x0064u);
        uint8_t b1 = (uint8_t)((Rng_NextMem(m) & 0x003Fu) - 0x0020u);
        uint32_t d1 = ((uint32_t)b0 << 24) | ((uint32_t)b1 << 16) | 0x0003u;
        (void)Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, d1);
        if ((rd16(m, obj + 0x06u) & 0x0007u) == 0u)
            (void)Obj_Spawn(m, obj, 0x0013EE08u, 5u, 2u, (d1 & 0xFFFF0000u) | 0x0600u);
    }
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x00124358u);
        return 0;
    }
    wr8(m, obj + 0x59u, 0xFFu);
    Obj_11AE2C(m, obj);
    return 0;
}
