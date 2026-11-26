/* ROM 0x0CC2DE: difficulty-scaled random modulo (D0.w = base range -> D0 = (quo<<16)|rem). */
#include "fn_auto.h"

int Fn_0CC2DE(const f3mem_t *m, f3regs_t *r)
{
    uint16_t d0w = (uint16_t)r->d[0];
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu)); /* 0x0CC2DE..0x0CC2E2 */
    uint32_t d1 = ((uint32_t)d0w * (uint32_t)d1w) << 8; /* 0x0CC2E6..0x0CC2E8 */
    d1 = (d1 >> 16) | (d1 << 16);                       /* 0x0CC2EA: swap D1 */
    if ((uint16_t)d1 == 0u)                             /* 0x0CC2EC */
        d1 = 1u;                                        /* 0x0CC2F2 */

    uint32_t d0 = Rng_NextMem(m) & 0x0000FFFFu;         /* 0x0CC2F4..0x0CC2FA */
    uint16_t div = (uint16_t)d1;
    uint16_t quo = (uint16_t)(d0 / div);                /* 0x0CC300 */
    uint16_t rem = (uint16_t)(d0 % div);

    r->d[0] = ((uint32_t)quo << 16) | rem;              /* 0x0CC302: swap D0 */
    r->d[1] = d1;
    return 0;
}
