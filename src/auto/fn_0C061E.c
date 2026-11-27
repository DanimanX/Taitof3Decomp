/* ROM 0x0C061E: clears the A5+0x8CE counter and frees the object (A0). */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_0C061E(const f3mem_t *m, f3regs_t *r)
{
    wr32(m, F3_A5 + 0x08CEu, 0u);                       /* 0x0C061E */
    r->a[7] -= 4;                                       /* 0x0C0622 */
    wr32(m, r->a[7], r->a[0]);
    Obj_Free(m, r->a[0]);                               /* 0x0C0624 -> 0x0105EC */
    r->a[7] += 4;                                       /* 0x0C062A */
    return 0;
}
