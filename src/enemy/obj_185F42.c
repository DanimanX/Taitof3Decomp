#include "obj_185F42.h"
#include "../objects/obj_pool.h"

/* ROM 0x185F42: subq.w #1,(0x6,A0) ; bgt 0x185F3A (re-arm) ; else free */
int Obj_185F42_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t n = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        wr32(m, obj + 0x00u, 0x00185F42u);          /* 0x185F3A */
        return 0;
    }
    Obj_Free(m, obj);                               /* 0x185F48 */
    return 0;
}
