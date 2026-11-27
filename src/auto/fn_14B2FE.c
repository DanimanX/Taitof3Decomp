#include "fn_auto.h"
/* ROM 0x14B2FE: UNVERIFIED (no capture): Return the current object to the pool. */
#include "../objects/obj_pool.h"

int Fn_14B2FE(const f3mem_t *m, f3regs_t *r)
{
    /* 0x14B2FE pushes A0, then 0x14B300 pushes the return address. */
    uint32_t sp = r->a[7];
    wr32(m, sp - 4u, r->a[0]);
    wr32(m, sp - 8u, 0x14B306u);
    /* 0x0105EC MOVEM predecrement saves A1, A0, D0 in that order. */
    wr32(m, sp - 12u, r->a[1]);
    wr32(m, sp - 16u, r->a[0]);
    wr32(m, sp - 20u, r->d[0]);
    Obj_Free(m, rd32(m, sp - 4u));
    r->d[0] = rd32(m, sp - 20u); /* 0x010608 */
    r->a[0] = rd32(m, sp - 16u);
    r->a[1] = rd32(m, sp - 12u);
    /* 0x14B306 discards the argument after the callee's RTS. */
    return 0;
}
