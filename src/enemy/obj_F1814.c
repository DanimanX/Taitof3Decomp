#include "obj_F1814.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x0F1814: enemy init */
int Obj_F1814_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_SeedFromParams(m, obj, 0x000F16ACu);
    Obj_ClassColour(m, obj);
    wr32(m, obj + 0x44u, 0x000F182Cu);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
    Obj_StartBehaviour(m, obj, 0x000F1762u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));
    wr32(m, obj + 0x00u, 0x000F1432u);
    return 0;
}
