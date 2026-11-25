#include "obj_17AE0A.h"
#include "../objects/obj_pool.h"

/* ROM 0x17AE0A: boss death trigger — set flags, record death position, reset
 * enemy/shot object groups (3..4, 5..6, 9..10), and free self. */
int Obj_17AE0A_Tick(const f3mem_t *m, uint32_t obj)
{
    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) | (1u << 3)));
    wr8(m, F3_A5 + 0x4B26u, (uint8_t)(rd8(m, F3_A5 + 0x4B26u) | 0x06u));

    uint32_t a4 = rd32(m, obj + 0x08u);
    wr16(m, F3_A5 + 0x32FAu, (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, a4 + 0x16u)));
    wr16(m, F3_A5 + 0x32FCu, (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, a4 + 0x1Au)));

    Obj_ResetGroups(m, 3u, 2u);
    Obj_ResetGroups(m, 5u, 2u);
    Obj_ResetGroups(m, 9u, 2u);
    Obj_Free(m, obj);
    return 0;
}
