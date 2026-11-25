#include "obj_190AA6.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x190CBA: spawn the per-kind satellites */
static void sub_190CBA(const f3mem_t *m, uint32_t obj)
{
    uint8_t k = rd8(m, obj + 0x22u);
    if (k == 7u) {
        Obj_Spawn(m, obj, 0x190FD0u, 5u, 2u, 0x1911A2u);
    } else if (k == 8u) {
        Obj_Spawn(m, obj, 0x190FD0u, 5u, 2u, 0x1911B2u);
    } else if (k == 9u) {
        Obj_Spawn(m, obj, 0x190FD0u, 5u, 2u, 0x1911C2u);
    } else if (k == 10u) {
        Obj_Spawn(m, obj, 0x190FD0u, 6u, 1u, 0x1911D2u);
        Obj_Spawn(m, obj, 0x190FD0u, 6u, 1u, 0x1911E6u);
        Obj_Spawn(m, obj, 0x190FD0u, 6u, 1u, 0x1911FAu);
        Obj_Spawn(m, obj, 0x191356u, 5u, 2u, 0u);
    }
}

/* ROM 0x190AA6: init one chain link from the 6-byte record
 * 0x190F8E[arg] = {anim.l, +0x42.b, +0x5C.b} */
int Obj_190AA6_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_17A7FA(m, obj);
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr8(m, obj + 0x22u, (uint8_t)d0);
    uint32_t a2 = 0x190F8Eu + (uint32_t)(int32_t)(int16_t)(uint16_t)((d0 & 0xFFFFu) * 6u);
    wr16(m, obj + 0x42u, 0u);
    wr16(m, obj + 0x5Cu, 0u);
    wr16(m, obj + 0x5Eu, 0u);
    wr16(m, obj + 0x60u, 0u);
    uint32_t a1 = rd32(m, a2);
    a2 += 4u;
    uint16_t idx = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, idx);
    wr32(m, obj + 0x3Au, a1);
    if (Obj_AnimInit(m, obj, idx) < 0)
        return -1;
    wr8(m, obj + 0x42u, rd8(m, a2));
    wr8(m, obj + 0x5Cu, rd8(m, a2 + 1u));
    wr8(m, obj + 0x34u, 0x10u);
    wr16(m, obj + 0x24u, 0u);
    wr16(m, obj + 0x28u, 1u);
    uint32_t a4 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x08u, rd32(m, a4 + 0x08u));
    sub_190CBA(m, obj);
    wr32(m, obj, 0x190B1Au);
    return 0;
}
