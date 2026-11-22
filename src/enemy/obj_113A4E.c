#include "obj_113A4E.h"
#include "../objects/obj_pool.h"

/* ROM 0x113A4E */
int Obj_113A4E_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);   /* move.l A0,-(SP); jsr 0x0105EC */
    return 0;
}
