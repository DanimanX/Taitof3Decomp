#include "obj_C7E10.h"
#include "../objects/obj_pool.h"

/* ROM 0x0C7E10 */
int Obj_C7E10_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Spawn(m, obj, 0x000C7F14u, 0x0Bu, 1u, 0x000C7FEEu);
    Obj_Spawn(m, obj, 0x000C8208u, 0x0Bu, 1u, 0u);
    wr16(m, obj + 0x06u, 0x010Cu);
    wr16(m, obj + 0x22u, 0x0007u);
    wr32(m, obj + 0x00u, 0x000C7E5Eu);
    return 0;
}
