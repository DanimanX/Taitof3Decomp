#include "obj_12EB0E.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../audio/sound_cmd.h"

/* ROM 0x12EB0E: bullet fired by the 0x12EA24 toaster; links itself into the
 * parent's sibling list (parent+0x48 chain, prev at +0x44), inherits the
 * parent's spawn index and starts the 0x12EB90 state. */
int Obj_12EB0E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x12EB0E: motion record {dy.w, dx.l} at obj+0x0C */
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    uint32_t a2 = rd32(m, a1);
    wr32(m, obj + 0x52u, rd32(m, a1 + 4u));
    wr16(m, obj + 0x50u, rd16(m, a2));
    a2 += 2u;
    wr32(m, obj + 0x3Au, a2);

    /* 0x12EB20: spawn index from the parent, then walk to the list tail */
    a1 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x22u, rd16(m, a1 + 0x22u));
    wr16(m, a1 + 0x22u, (uint16_t)(rd16(m, a1 + 0x22u) + 1u));
    wr32(m, obj + 0x44u, 0u);
    wr32(m, obj + 0x48u, 0u);
    wr32(m, obj + 0x62u, 0u);
    wr32(m, obj + 0x66u, 0u);
    a2 = a1;
    for (;;) {
        uint32_t d0 = rd32(m, a2 + 0x48u);
        if (d0 == 0u)
            break;
        a2 = d0;
    }
    wr32(m, obj + 0x44u, a2);
    wr32(m, a2 + 0x48u, obj);
    wr32(m, obj + 0x62u, a2);
    wr32(m, a2 + 0x66u, obj);

    /* 0x12EB5E */
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x28u, 0x00010000u);
    wr32(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x20u, rd16(m, F3_A5 + 0x32A2u));
    wr8(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x10u, 0x00028740u);
    wr32(m, obj + 0x00u, 0x0012EB90u);
    return 0;
}
