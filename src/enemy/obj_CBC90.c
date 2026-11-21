#include "obj_CBC90.h"
#include "../objects/obj_pool.h"

/* ROM 0xCBC90 */
int Obj_CBC90_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0CBC90: move.l #0xCBC98,(A0) */
    wr32(m, obj, 0x000CBC98u);
    return 0;
}
