#include "obj_1000DE.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x1000DE: init enemy stats + behaviour, arm 0x10010C */
int Obj_1000DE_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_0CEDE4(m, obj, 0x00100228u);
    Obj_StartBehaviour(m, obj, 0x00100248u);
    wr8(m, obj + 0x1Du, 0xEEu);
    wr8(m, obj + 0x4Fu, 0xEEu);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
    wr32(m, obj + 0x00u, 0x0010010Cu);
    return 0;
}
