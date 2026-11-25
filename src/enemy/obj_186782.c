#include "obj_186782.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x184908: seed object fields and animation from descriptor at +0x0C. */
static int sub_184908(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);

    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x34u, 0u);

    wr16(m, obj + 0x24u, rd16(m, a1));
    a1 += 2u;
    wr16(m, obj + 0x28u, rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1));
    a1 += 4u;
    wr16(m, obj + 0x16u, rd16(m, a1));
    a1 += 2u;
    wr16(m, obj + 0x1Au, rd16(m, a1));
    a1 += 2u;

    a1 = rd32(m, a1);
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, d0);
    wr32(m, obj + 0x3Au, a1);

    if (d0 == 1u) {
        /* ROM 0x1795F6 / 0x179880: (+0x20 + 8) >> 4 indexes 4-byte sprite ptrs. */
        wr16(m, obj + 0x3Eu, 1u);
        uint16_t idx = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 8u) >> 4);
        wr32(m, obj + 0x10u, rd32(m, a1 + (uint32_t)idx * 4u));
    } else if (Obj_AnimInit(m, obj, d0) < 0) {
        return -1;
    }

    Obj_18495E(m, obj);
    wr16(m, obj + 0x62u, 0u);
    wr32(m, obj + 0x64u, 0u);
    wr32(m, obj + 0x68u, 0u);
    return 0;
}

/* ROM 0x186782: boss part init via 0x184908, then install 0x1867A4. */
int Obj_186782_Tick(const f3mem_t *m, uint32_t obj)
{
    if (sub_184908(m, obj) < 0)
        return -1;

    uint32_t a3 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x34u, rd16(m, a3 + 0x20u));
    wr16(m, obj + 0x22u, rd16(m, a3 + 0x22u));
    wr32(m, obj + 0x00u, 0x001867A4u);
    return 0;
}
