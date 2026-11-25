#include "obj_186406.h"
#include "../objects/obj_anim.h"

/* ROM 0x186406: boss body segment init. Indexes the 16-byte record table at
 * 0x18652E by +0x0C, runs the 0x179A5C anim init, seeds hitboxes/angles, and
 * installs 0x186482. */
int Obj_186406_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr8(m, obj + 0x22u, (uint8_t)d0);

    uint32_t a2 = 0x0018652Eu + (uint32_t)((uint16_t)d0 * 0x10u);
    wr16(m, obj + 0x42u, 0u);
    wr16(m, obj + 0x5Cu, 0u);
    wr16(m, obj + 0x5Eu, 0u);
    wr16(m, obj + 0x60u, 0u);

    uint32_t a1 = rd32(m, a2);
    a2 += 4u;
    uint16_t anim_idx = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, anim_idx);
    wr32(m, obj + 0x3Au, a1);

    if (Obj_AnimInit(m, obj, anim_idx) < 0)
        return -1;

    wr32(m, obj + 0x2Cu, rd32(m, a2));
    a2 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a2));
    a2 += 4u;
    wr8(m, obj + 0x42u, rd8(m, a2));
    a2 += 1u;
    wr8(m, obj + 0x5Cu, rd8(m, a2));
    a2 += 1u;
    wr8(m, obj + 0x5Eu, rd8(m, a2));
    a2 += 1u;
    wr8(m, obj + 0x0Cu, rd8(m, a2));

    wr16(m, obj + 0x1Cu, 0u);
    wr8(m, obj + 0x35u, 0u);
    wr8(m, obj + 0x34u, 0x1Au);
    wr32(m, obj + 0x24u, 0x00010000u);
    wr32(m, obj + 0x28u, 0x00010000u);
    wr32(m, obj + 0x00u, 0x00186482u);
    return 0;
}
