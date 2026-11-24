#include "obj_116C48.h"
#include "../objects/obj_common.h"

/* ROM 0x116C48: register a group child under the parent (obj+8): copy the
 * parent's index, bump it, seed from the A5 globals and link into the parent's
 * +0x50 slot. The +0x54 store is word-wide only. */
int Obj_116C48_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    wr16(m, obj + 0x28u, 0x0001u);
    wr16(m, obj + 0x2Cu, (uint16_t)-0x4);
    wr16(m, obj + 0x30u, (uint16_t)-0x4);
    wr16(m, obj + 0x2Eu, 0x0004u);
    wr16(m, obj + 0x32u, 0x0004u);
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x22u, rd16(m, a1 + 0x22u));
    wr16(m, a1 + 0x22u, (uint16_t)(rd16(m, a1 + 0x22u) + 1u)); /* addq.w #1 */
    wr16(m, obj + 0x20u, rd16(m, F3_A5 + 0x30BCu));
    wr16(m, obj + 0x54u, rd16(m, F3_A5 + 0x30BEu));
    wr32(m, obj + 0x50u, 0u);
    a1 = rd32(m, obj + 0x0Cu);
    wr32(m, a1 + 0x50u, obj);
    wr32(m, obj + 0x00u, 0x116C9Au);
    return 0;
}
