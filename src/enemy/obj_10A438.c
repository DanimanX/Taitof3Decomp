#include "obj_10A438.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x10A438: enemy init */
int Obj_10A438_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_SeedFromParams(m, obj, 0x0010A726u);
    Obj_ClassColour(m, obj);
    uint16_t idx = rd16(m, obj + 0x10u);
    wr32(m, obj + 0x44u, rd32(m, 0x0010A712u + (uint32_t)idx * 4u));
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
    Obj_StartBehaviour(m, obj, 0x0010AA22u);
    wr8(m, obj + 0x06u, 1u);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
    wr8(m, obj + 0x34u, (uint8_t)((rd8(m, obj + 0x34u) & ~0x02u) | 0x04u));
    wr32(m, obj + 0x00u, 0x0010A484u);
    return 0;
}
