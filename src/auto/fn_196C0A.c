#include "fn_auto.h"

/* ROM 0x196C0A: UNVERIFIED (weak capture): keep the object while its parent's health is positive. */
#include "../objects/obj_pool.h"

int Fn_196C0A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    r->a[4] = rd32(m, obj + 8u);
    if ((int32_t)rd32(m, r->a[4] + 0x24u) > 0) {
        wr32(m, obj, 0x196C0Au); /* 0x196C02 */
    } else {
        uint32_t sp = r->a[7];
        wr32(m, sp - 4u, obj); /* 0x196C14 */
        wr32(m, sp - 8u, 0x196C1Cu);
        /* 0x0105EC saves A1, A0 and D0, then restores them. */
        wr32(m, sp - 12u, r->a[1]);
        wr32(m, sp - 16u, obj);
        wr32(m, sp - 20u, r->d[0]);
        Obj_Free(m, rd32(m, sp - 4u));
    }
    r->a[7] += 4u;
    return 0;
}
