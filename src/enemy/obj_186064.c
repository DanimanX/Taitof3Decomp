#include "obj_186064.h"
#include "../objects/obj_anim.h"

/* ROM 0x186064: init child from 12-byte table at 0x18616E[+0x0C] and hand off to 0x1860AA. */
int Obj_186064_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr8(m, obj + 0x22u, (uint8_t)d0);
    uint32_t entry = 0x0018616Eu + (uint32_t)(uint16_t)d0 * 12u;

    uint32_t anim_ptr = rd32(m, entry);
    uint16_t anim_idx = rd16(m, anim_ptr);
    anim_ptr += 2u;
    wr16(m, obj + 0x40u, anim_idx);
    wr32(m, obj + 0x3Au, anim_ptr);
    if (Obj_AnimInit(m, obj, anim_idx) < 0)
        return -1;

    wr16(m, obj + 0x16u, rd16(m, entry + 4u));
    wr16(m, obj + 0x1Au, rd16(m, entry + 6u));
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr32(m, obj + 0x00u, 0x001860AAu);
    return 0;
}
