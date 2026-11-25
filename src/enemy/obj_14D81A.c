#include "obj_14D81A.h"
#include "../objects/obj_common.h"

/* ROM 0x14D81A: init scroll/line-RAM state and arm 0x14D86E */
int Obj_14D81A_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, F3_A5 + 0x422Cu, 0u);
    wr32(m, F3_A5 + 0x4230u, 0u);
    Mem_WritePair64(m, F3_A5 + 0x420Cu, F3_A5 - 0x5DA0u, 0x00180000u, 0u);
    wr32(m, obj + 0x44u, 0u);
    wr16(m, obj + 0x06u, 9u);
    wr32(m, obj + 0x00u, 0x0014D86Eu);
    return 0;
}
