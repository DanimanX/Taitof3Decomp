#include "obj_116EE4.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x116EE4 */
int Obj_116EE4_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    if (rd8(m, a1 + 0x50u) & 0x20u) {
        Obj_Free(m, obj);
        return 0;
    }
    Obj_116F12(m, rd32(m, F3_A5 + 0x30B6u));
    Obj_116F12(m, rd32(m, F3_A5 + 0x30B2u));
    wr32(m, obj, 0x00116EE4u);
    return 0;
}
