#include "obj_1114F6.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

/* ROM 0x1114F6: seed the draw fields from the spawn argument, then chain on */
int Obj_1114F6_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    uint32_t a2 = rd32(m, obj + 0x0Cu);
    wr32(m, obj + 0x18u, rd32(m, a2 + 0x18u));
    wr32(m, obj + 0x14u, rd32(m, a2 + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x10u));
    wr32(m, obj + 0x00u, 0x11151Au);
    return 0;
}
