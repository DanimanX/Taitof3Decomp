#include "obj_F241C.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x0F241C: enemy init */
int Obj_F241C_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_SeedFromParams(m, obj, 0x000F2750u);
    Obj_ClassColour(m, obj);
    wr32(m, obj + 0x44u, rd32(m, obj + 0x10u));
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
    Obj_StartBehaviour(m, obj, 0x000F2824u);
    wr8(m, obj + 0x34u, (uint8_t)((rd8(m, obj + 0x34u) & ~0x02u) | 0x04u));
    wr32(m, obj + 0x00u, 0x000F2454u);
    return 0;
}
