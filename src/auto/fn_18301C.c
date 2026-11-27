#include "fn_auto.h"

/* ROM 0x18301C: UNVERIFIED (weak capture): free the object when the global flag is set. */
#include "../objects/obj_pool.h"

int Fn_18301C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (!(rd8(m, r->a[5] + 0x1D58u) & 2u)) {
        wr32(m, obj, 0x18301Cu); /* 0x183014 */
    } else {
        uint32_t sp = r->a[7];
        wr32(m, sp - 4u, obj); /* 0x183024 */
        wr32(m, sp - 8u, 0x18302Cu);
        /* 0x0105EC saves A1, A0 and D0, then restores them. */
        wr32(m, sp - 12u, r->a[1]);
        wr32(m, sp - 16u, obj);
        wr32(m, sp - 20u, r->d[0]);
        Obj_Free(m, rd32(m, sp - 4u));
    }
    r->a[7] += 4u;
    return 0;
}
