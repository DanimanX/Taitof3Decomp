#include "obj_190FD0.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x190FD0: init boss sub-part from +0x0C descriptor, arm 0x191014 */
int Obj_190FD0_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_17A7FA(m, obj);
    uint32_t a3 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x22u, rd16(m, a3)); a3 += 2u;
    uint32_t a1 = rd32(m, a3);
    uint16_t d0 = rd16(m, a1); a1 += 2u;
    wr16(m, obj + 0x40u, d0);
    wr32(m, obj + 0x3Au, a1);
    if (Obj_AnimInit(m, obj, d0) < 0)
        return -1;
    wr32(m, obj + 0x0Cu, a3);
    wr16(m, obj + 0x24u, 1u);
    wr16(m, obj + 0x28u, 1u);
    wr8(m, obj + 0x34u, 0x0Au);
    wr32(m, obj + 0x00u, 0x00191014u);
    return 0;
}
