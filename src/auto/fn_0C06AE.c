/* ROM 0x0C06AE: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

static void FUN_000cbd86(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x0CBD86: Clear words/longs at A0 offsets */
    wr16(m, r->a[0] + 0x1cu, 0u);
    wr16(m, r->a[0] + 0x1eu, 0u);
    wr16(m, r->a[0] + 0x34u, 0u);
    wr16(m, r->a[0] + 0x3au, 0u);
    wr16(m, r->a[0] + 0x3eu, 0u);
    wr32(m, r->a[0] + 0x24u, 0u);
    wr32(m, r->a[0] + 0x28u, 0u);
}

int Fn_0C06AE(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x0C06AE: jsr 0x000cbd86.l */
    r->a[7] -= 4;
    wr32(m, r->a[7], 0x000C06B4u);
    FUN_000cbd86(m, r);
    r->a[7] += 4;

    /* ROM 0x0C06B4: move.l #0xc0746,(0x5c,A0) */
    wr32(m, r->a[0] + 0x5cu, 0x000c0746u);

    /* ROM 0x0C06BC: move.b #-0x5c,(0x1d,A0) */
    wr8(m, r->a[0] + 0x1du, 0xA4u);

    /* ROM 0x0C06C2: move.l #0x0,(0x14,A0) */
    wr32(m, r->a[0] + 0x14u, 0u);

    /* ROM 0x0C06CA: move.l #0x400000,(0x18,A0) */
    wr32(m, r->a[0] + 0x18u, 0x400000u);

    /* ROM 0x0C06D2: move.w #0x8c,(0x60,A0) */
    wr16(m, r->a[0] + 0x60u, 0x8cu);

    /* ROM 0x0C06D8: move.l #0xc06e0,(A0) */
    wr32(m, r->a[0], 0x000c06e0u);

    /* ROM 0x0C06DE: rts */
    return 0;
}
