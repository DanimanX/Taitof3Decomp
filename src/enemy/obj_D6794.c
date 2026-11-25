#include "obj_D6794.h"
#include "../objects/obj_pool.h"

/* ROM 0x0D6794: one-shot -- free the object on its first tick and return. */
int Obj_D6794_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0D6794: move.l A0,-(SP) ; jsr 0x105EC ; lea (0x4,SP),SP ; rts */
    Obj_Free(m, obj);
    return 0;
}
