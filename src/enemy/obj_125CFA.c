#include "obj_125CFA.h"
#include "../objects/obj_pool.h"

/* ROM 0x125CFA: free the object (one-shot body). */
int Obj_125CFA_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x125CFA: move.l A0,-(SP); jsr 0x105EC; lea (4,SP),SP; rts */
    Obj_Free(m, obj);
    return 0;
}
