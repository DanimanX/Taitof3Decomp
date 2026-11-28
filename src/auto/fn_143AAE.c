/* ROM 0x143AAE: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

static void FUN_000cbd86(const f3mem_t *m, f3regs_t *r) {
    (void)m; (void)r;
    /* ROM 0x0CBD86: Clear words/longs at A0 offsets */
    wr16(m, r->a[0] + 0x1cu, 0u);
    wr16(m, r->a[0] + 0x1eu, 0u);
    wr16(m, r->a[0] + 0x34u, 0u);
    wr16(m, r->a[0] + 0x3au, 0u);
    wr16(m, r->a[0] + 0x3eu, 0u);
    wr32(m, r->a[0] + 0x24u, 0u);
    wr32(m, r->a[0] + 0x28u, 0u);
}

int Fn_143AAE(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x143AAE: Call FUN_000cbd86 */
    r->a[7] -= 4;
    wr32(m, r->a[7], 0x143AB4u);
    FUN_000cbd86(m, r);
    r->a[7] += 4;

    /* ROM 0x143AB4: Store 0x5a4e8 to A0+16 */
    wr32(m, r->a[0] + 0x10u, 0x5a4e8u);

    /* ROM 0x143ABC: Store 0x143ac4 to A0 */
    wr32(m, r->a[0], 0x143ac4u);

    return 0;
}
