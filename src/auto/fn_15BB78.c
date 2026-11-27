/* ROM 0x15BB78: frees the object (A0) via 0x0105EC. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_15BB78(const f3mem_t *m, f3regs_t *r)
{
    r->a[7] -= 4;                                       /* 0x15BB78 */
    wr32(m, r->a[7], r->a[0]);
    Obj_Free(m, r->a[0]);                               /* 0x15BB7A -> 0x0105EC */
    r->a[7] += 4;                                       /* 0x15BB80 */
    return 0;
}
