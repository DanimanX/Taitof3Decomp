#include "obj_F75DE.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x0F75DE: init enemy from +0x12 table, stats, behaviour, 4-way rng timer */
int Obj_F75DE_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t idx = rd16(m, obj + 0x12u);
    wr32(m, obj + 0x0Cu, rd32(m, 0x000F766Eu + (uint32_t)idx * 4u));
    Obj_0CEDE4(m, obj, 0x000F8214u);
    Obj_StartBehaviour(m, obj, 0x000F76E6u);
    wr16(m, obj + 0x4Cu, (uint16_t)((Obj_Next4Way(m) & 7u) + 1u));
    wr32(m, obj + 0x00u, 0x000F7618u);
    return 0;
}
