/* ROM 0x1121A0: free an object (A0 = object) via Obj_Free (0x0105EC). */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_1121A0(const f3mem_t *m, f3regs_t *r)
{
    Obj_Free(m, r->a[0]);                            /* 0x1121A0 push A0 ; 0x1121A2 jsr 0x105EC */
    return 0;
}
