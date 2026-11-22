#include "obj_115D14.h"
#include "../objects/obj_pool.h"

/* ROM 0x115D14 */
int Obj_115D14_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);   /* move.l A0,-(SP); jsr 0x0105EC */
    return 0;
}
