/* ROM 0x11235A: free the object in A0 (push A0 ; jsr 0x0105EC). */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_11235A(const f3mem_t *m, f3regs_t *r)
{
    Obj_Free(m, r->a[0]);                                /* 0x11235C -> 0x0105EC */
    return 0;
}
