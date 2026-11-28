/* ROM 0x19F270: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_19F270(const f3mem_t *m, f3regs_t *r) {
    (void)m;
    /* ROM 0x19F270: Push D0, A1, A2; copy 9 bytes from 0x19F28E to 0x40CBC4; pop D0, A1, A2; RTS */
    r->a[7] -= 12;
    wr32(m, r->a[7], r->d[0]);
    wr32(m, r->a[7] + 4, r->a[1]);
    wr32(m, r->a[7] + 8, r->a[2]);

    r->a[1] = 0x40CBC4u;
    r->a[2] = 0x19F28Eu;

    for (int i = 0; i < 9; i++) {
        uint8_t b = rd8(m, r->a[2]);
        r->a[2]++;
        wr8(m, r->a[1], b);
        r->a[1]++;
    }

    r->d[0] = rd32(m, r->a[7]);
    r->a[1] = rd32(m, r->a[7] + 4);
    r->a[2] = rd32(m, r->a[7] + 8);
    r->a[7] += 12;
    return 0;
}
