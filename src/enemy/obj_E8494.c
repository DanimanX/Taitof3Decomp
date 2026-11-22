#include "obj_E8494.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

/* ROM 0x0E8494: spawner, emits 0xE81A0 every 20 frames, count in +0x21 */
int Obj_E8494_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u || rd8(m, obj + 0x21u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, t);
    if (t != 0u)
        return 0;
    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 1u));
    wr8(m, obj + 0x23u, 0x14u);
    uint32_t c = Obj_Spawn(m, obj, 0xE81A0u, 5u, 2u, 0u);
    if ((int32_t)c < 0)
        return 0;
    wr32(m, c + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, c + 0x18u, rd32(m, obj + 0x18u));
    return 0;
}
