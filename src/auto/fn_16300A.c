/* ROM 0x16300A: free an object (A0 = object) via Obj_Free (0x0105EC). */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_16300A(const f3mem_t *m, f3regs_t *r)
{
    Obj_Free(m, r->a[0]);                            /* 0x16300A push A0 ; 0x16300C jsr 0x105EC */
    return 0;
}
