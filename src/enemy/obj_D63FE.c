#include "obj_D63FE.h"
#include "../objects/obj_pool.h"

int Obj_D63FE_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0D63FE: move.l A0,-(SP) */
    /* 0x0D6400: jsr 0x000105ec.l */
    (void)Obj_Free(m, obj);
    /* 0x0D6406: lea (0x4,SP),SP */
    /* 0x0D640A: rts */
    return 0;
}
