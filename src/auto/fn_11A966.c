/* ROM 0x11A966: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

/* ROM 0x11A95E: Store 0x11A966 to (A0) */
static void Fn_11A95E(const f3mem_t *m, f3regs_t *r) {
    wr32(m, r->a[0], 0x11A966);
}

/* ROM 0x0CBD86: Clear object fields for initialization */
static void Fn_0CBD86(const f3mem_t *m, f3regs_t *r) {
    /* 0CBD86: clr.w (0x1C,A0) */
    wr16(m, r->a[0] + 0x1Cu, 0u);
    /* 0CBD8A: clr.w (0x1E,A0) */
    wr16(m, r->a[0] + 0x1Eu, 0u);
    /* 0CBD8E: clr.w (0x34,A0) */
    wr16(m, r->a[0] + 0x34u, 0u);
    /* 0CBD92: clr.w (0x3A,A0) */
    wr16(m, r->a[0] + 0x3Au, 0u);
    /* 0CBD96: clr.w (0x3E,A0) */
    wr16(m, r->a[0] + 0x3Eu, 0u);
    /* 0CBD9A: clr.l (0x24,A0) */
    wr32(m, r->a[0] + 0x24u, 0u);
    /* 0CBD9E: clr.l (0x28,A0) */
    wr32(m, r->a[0] + 0x28u, 0u);
}

int Fn_11A966(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x11A966: subq.w #1, (6,A0) */
    uint16_t val = rd16(m, r->a[0] + 6u);
    val--;
    wr16(m, r->a[0] + 6u, val);

    /* ROM 0x11A96A: bgt.b 0x11A95E (if val > 0 signed) */
    if ((int16_t)val > 0) {
        r->a[7] -= 4;
        wr32(m, r->a[7], 0x11A96C);
        Fn_11A95E(m, r);
        r->a[7] += 4;
        /* Return immediately after the branch target returns */
        return 0;
    }

    /* ROM 0x11A96C: jsr 0x0CBD86 */
    r->a[7] -= 4;
    wr32(m, r->a[7], 0x0CBD86);
    Fn_0CBD86(m, r);
    r->a[7] += 4;

    /* ROM 0x11A972: movea.l (8,A0), A1 */
    r->a[1] = rd32(m, r->a[0] + 8u);

    /* ROM 0x11A976: move.l (18,A1), (18,A0) */
    wr32(m, r->a[0] + 0x18u, rd32(m, r->a[1] + 0x18u));

    /* ROM 0x11A97C: move.l #-0x200000, (14,A0) */
    wr32(m, r->a[0] + 0x14u, 0xFFE00000u);

    /* ROM 0x11A984: clr.w (22,A0) */
    wr16(m, r->a[0] + 0x22u, 0u);

    /* ROM 0x11A988: clr.w (6,A0) */
    wr16(m, r->a[0] + 6u, 0u);

    /* ROM 0x11A98C: move.l #0x11A994, (A0) */
    wr32(m, r->a[0], 0x11A994);

    /* ROM 0x11A992: rts */
    return 0;
}
