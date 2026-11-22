#include "obj_E94BC.h"
#include "obj_E8B72.h"
#include "../objects/obj_common.h"

/* ROM 0x0E94BC: hit box riding on the grandparent */
int Obj_E94BC_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, rd32(m, obj + 0x08u) + 0x08u);
    if (rd8(m, a1 + 0x06u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        wr8(m, a1 + 0x06u, 0x05u);
        Score_OnHit(m, obj, 0x00000190u);
        Obj_StartDying(m, obj);
        return 0;
    }
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    Coll_AddEnemyHazard(m, obj);
    return 0;
}
