/* ROM 0x0D8CEE: free an object (A0 = object) via Obj_Free (0x0105EC). */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_0D8CEE(const f3mem_t *m, f3regs_t *r)
{
    Obj_Free(m, r->a[0]);                            /* 0x0D8CEE push A0 ; 0x0D8CF0 jsr 0x105EC */
    return 0;
}
