#include "obj_CB406.h"
#include "../objects/obj_pool.h"

/* ROM 0xCB406 */
int Obj_CB406_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0CB406: sf (0x22,A0) */
    wr8(m, obj + 0x22u, 0u);
    /* 0x0CB40A: move.l #0xCB412,(A0) */
    wr32(m, obj, 0x000CB412u);
    return 0;
}
