#include "fn_auto.h"
/* ROM 0x13AD8A: verified on a poke-forced capture (scripts/zcap_poke_weak.sh): wait for stage phase two, initialize the next state, or free the object. */
#include "../objects/obj_pool.h"

int Fn_13AD8A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a5 = r->a[5];
    if ((rd8(m, a5 - 0x144Bu) & 0x10u) ||
        (int16_t)rd16(m, a5 + 0x32D6u) >= 3) {
        /* 0x13B062: push the argument and return address, then 0x0105EC MOVEM. */
        uint32_t sp = r->a[7];
        wr32(m, sp - 4u, obj);
        wr32(m, sp - 8u, 0x13B06Au);
        wr32(m, sp - 12u, r->a[1]);
        wr32(m, sp - 16u, obj);
        wr32(m, sp - 20u, r->d[0]);
        Obj_Free(m, obj);
    } else if (rd16(m, a5 + 0x32D6u) != 2u) {
        wr32(m, obj, 0x13AD8Au); /* 0x13AD82 */
    } else {
        wr16(m, obj + 0x34u, 0u);
        wr32(m, obj + 0x24u, 0x000F0000u);
        wr16(m, obj + 0x1Eu, 0x8080u);
        wr16(m, obj + 0x22u, 0x0400u);
        wr8(m, obj + 0x1Du, 0xC8u);
        wr32(m, obj + 0x10u, 0x03B7F8u);
        wr32(m, obj, 0x13ADD4u); /* 0x13ADCC */
    }
    r->a[7] += 4u;
    return 0;
}
