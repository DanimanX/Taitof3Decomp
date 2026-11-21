#include "obj_12EA9E.h"
#include "../objects/obj_pool.h"

/* ROM 0x12EA9E: wait +0x06 frames, then spawn 0x12EF24 */
int Obj_12EA9E_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t n = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        wr32(m, obj + 0x00u, 0x0012EA9Eu);
        return 0;
    }
    Obj_Spawn(m, obj, 0x0012EF24u, 4u, 1u, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x1Du, 0x00u);   /* sf */
    wr8(m, obj + 0x4Fu, 0x00u);   /* sf */
    wr32(m, obj + 0x00u, 0x0012EAD4u);
    return 0;
}
