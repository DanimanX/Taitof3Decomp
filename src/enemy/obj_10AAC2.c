#include "obj_10AAC2.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

/* ROM 0x10AAC2: spawner, +0x21 children (0x10AB30) every 0x0F frames */
int Obj_10AAC2_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u || rd8(m, obj + 0x22u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if (rd8(m, obj + 0x21u) == 0u)
        return 0;
    uint8_t c23 = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, c23);
    if (c23 != 0u)
        return 0;
    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 1u));
    wr8(m, obj + 0x23u, 0x0Fu);
    uint32_t a1 = Obj_Spawn(m, obj, 0x0010AB30u, 5u, 2u, 0u);
    if ((int32_t)a1 < 0) {
        wr8(m, obj + 0x22u, (uint8_t)(rd8(m, obj + 0x22u) - 1u));
        return 0;
    }
    wr8(m, a1 + 0x4Eu, 0u);
    if (rd8(m, obj + 0x21u) == 0u)
        wr8(m, a1 + 0x4Eu, rd8(m, obj + 0x4Eu));
    return 0;
}
