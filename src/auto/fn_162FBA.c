/* ROM 0x162FBA: countdown tick clearing A5+0x4284 via 0x16AB58 (A0 = object). */
#include "fn_auto.h"

int Fn_162FBA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);            /* 0x162FBA */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                          /* 0x162FBE */
        wr32(m, obj, 0x00162FBAu);                                   /* 0x162FB2 */
        return 0;
    }

    r->d[0] &= 0xFFFFFF00u;                                          /* 0x162FC8 move.b #0,D0 */
    wr8(m, F3_A5 + 0x4284u, 0u);                                     /* 0x162FCC -> 0x16AB58 */
    wr16(m, obj + 0x06u, 0x0100u);                                   /* 0x162FD6 (beq never taken) */
    wr32(m, obj, 0x00162FE6u);                                       /* 0x162FDE */
    return 0;
}
