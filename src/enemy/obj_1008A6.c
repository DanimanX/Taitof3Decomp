#include "obj_1008A6.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x1008A6: enemy init */
int Obj_1008A6_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_SeedFromParams(m, obj, 0x001086FCu);
    uint16_t idx = (uint16_t)(Obj_Next4Way(m) & 7u);
    Obj_StartBehaviour(m, obj, rd32(m, 0x0010093Au + (uint32_t)idx * 4u));
    int16_t dx = (int16_t)((Obj_Next4Way(m) & 0x3Fu) - 0x20);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + dx));
    wr32(m, obj + 0x00u, 0x001008E2u);
    return 0;
}
