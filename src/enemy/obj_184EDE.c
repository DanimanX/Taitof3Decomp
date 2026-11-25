#include "obj_184EDE.h"
#include "../objects/obj_pool.h"

/* ROM 0x184EDE: countdown wait, then free 4 boss part objects and clear flag. */
int Obj_184EDE_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {
        wr32(m, obj + 0x00u, 0x00184EDEu);
        return 0;
    }

    Obj_Free(m, rd32(m, F3_A5 + 0x4B66u));
    Obj_Free(m, rd32(m, F3_A5 + 0x4B5Eu));
    Obj_Free(m, rd32(m, F3_A5 + 0x4B56u));
    Obj_Free(m, rd32(m, F3_A5 + 0x4B5Au));
    wr8(m, F3_A5 + 0x4B3Bu, (uint8_t)(rd8(m, F3_A5 + 0x4B3Bu) & ~0x02u));
    wr32(m, obj + 0x00u, 0x00184D6Cu);
    return 0;
}
