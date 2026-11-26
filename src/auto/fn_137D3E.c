/* ROM 0x137D3E: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_137D3E(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x137D3E: subq.w #1, (0x30d2,A5) */
    uint16_t val = rd16(m, F3_A5 + 0x30d2u);
    val--;
    wr16(m, F3_A5 + 0x30d2u, val);

    /* ROM 0x137D42: cmpi.w #-0xf0, (0x30d2,A5) */
    /* ROM 0x137D48: bgt.b 0x137D36 (branch if val > -0xf0) */
    if ((int16_t)val > -0xf0) {
        /* ROM 0x137D36: move.l #0x137d3e, (A0) */
        wr32(m, r->a[0], 0x00137D3Eu);
        /* ROM 0x137D3C: rts */
        return 0;
    }

    /* ROM 0x137D4A: move.w #0xb4, (0x6,A0) */
    wr16(m, r->a[0] + 6u, 0x00B4u);

    /* ROM 0x137D50: beq.b 0x137D60 (never taken, Z=0) */
    /* ROM 0x137D52: move.l #0x137d5a, (A0) */
    wr32(m, r->a[0], 0x00137D5Au);

    /* ROM 0x137D58: rts */
    return 0;
}
