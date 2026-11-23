#include "obj_EC65E.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
/* ROM 0x0D0E3A */

/* ROM 0xEC65E: spawner, emits 0xEC6FC every 0x14 frames (count +0x21, lives +0x22) */
int Obj_EC65E_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u || rd8(m, obj + 0x22u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if (rd8(m, obj + 0x21u) == 0u)
        return 0;
    uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, t);
    if (t != 0u)
        return 0;
    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 1u));
    wr8(m, obj + 0x23u, 0x14u);
    uint32_t c = Obj_Spawn(m, obj, 0xEC6FCu, 5u, 2u, 0u);
    if ((int32_t)c < 0) {
        wr8(m, obj + 0x22u, (uint8_t)(rd8(m, obj + 0x22u) - 1u));
        return 0;
    }
    wr32(m, c + 0x14u, rd32(m, obj + 0x14u));
    wr16(m, c + 0x14u, (uint16_t)(rd16(m, c + 0x14u) + (uint16_t)((Obj_Next4Way(m) & 0x3Fu) - 0x20u)));
    wr32(m, c + 0x18u, rd32(m, obj + 0x18u));
    wr16(m, c + 0x18u, (uint16_t)(rd16(m, c + 0x18u) + (uint16_t)((Obj_Next4Way(m) & 0x3Fu) - 0x20u)));
    wr8(m, c + 0x4Eu, 0u);
    if (rd8(m, obj + 0x21u) == 0u)
        wr8(m, c + 0x4Eu, rd8(m, obj + 0x4Eu));
    return 0;
}
