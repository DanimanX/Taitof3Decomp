#include "obj_BCF08.h"
#include "../objects/obj_common.h"

/* ROM 0x0BCF08: shrapnel bit spawned by 0x0BC70A (script 0x0BCFAC, velocity
 * from the spawn argument at obj+0x0C, 0xB frames). */
int Obj_BCF08_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    wr32(m, obj + 0x48u, 0x000BCFACu);
    wr32(m, obj + 0x44u, rd32(m, obj + 0x0Cu));
    wr8(m, obj + 0x1Cu, 0x01u);
    wr16(m, obj + 0x60u, 0x000Bu);
    wr32(m, obj + 0x00u, 0x000BCF30u);
    return 0;
}
