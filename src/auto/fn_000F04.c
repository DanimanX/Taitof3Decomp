#include "fn_auto.h"
/* ROM 0x000F04: UNVERIFIED (weak capture): update the coin-channel hold timer. */

int Fn_000F04(const f3mem_t *m, f3regs_t *r)
{
    uint32_t timer = r->a[0];
    if ((r->d[1] & (1u << (r->d[2] & 31u))) || !rd8(m, r->a[5] - 0x5DC7u)) {
        wr8(m, timer, 0);
    } else {
        uint8_t next = rd8(m, timer) + 1u;
        /* 0x000F14 branches to 0x000F3E, then the non-returning hardware halt. */
        if (next > 0x1Eu)
            return -1;
        wr8(m, timer, next);
    }
    r->a[7] += 4;
    return 0;
}
