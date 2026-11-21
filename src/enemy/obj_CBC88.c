#include "obj_CBC88.h"
#include "../objects/obj_pool.h"

/* ROM 0xCBC88 */
int Obj_CBC88_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0CBC88: move.l #0xCBC90,(A0) */
    wr32(m, obj, 0x000CBC90u);
    return 0;
}
