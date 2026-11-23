#include "obj_11A87E.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x11A87E */
int Obj_11A87E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x11A87E */
    Obj_ResetFields(m, obj);

    /* 0x11A884 */
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x2Cu, 0u);
    wr32(m, obj + 0x30u, 0xFFF80000u);   /* move.l #-0x80000 */
    wr16(m, obj + 0x06u, 0u);
    wr32(m, obj, 0x0011A8A8u);
    return 0;
}
