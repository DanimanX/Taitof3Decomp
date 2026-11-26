/* ROM 0x1333C2: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

static void sub_1331AC(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x1331AC: bfset (0x30ee,A5){0xb:0x1} */
    uint32_t bf_addr = r->a[5] + 0x30EEu;
    uint8_t v = rd8(m, bf_addr + 1u);
    v |= (uint8_t)(1u << 3u);
    wr8(m, bf_addr + 1u, v);

    /* ROM 0x1331B2: move.b #0x7,(0x30f2,A5) */
    wr8(m, r->a[5] + 0x30F2u, 7u);

    /* ROM 0x1331B8: bfset (0x30ee,A5){0xc:0x1} */
    v = rd8(m, bf_addr + 1u);
    v |= (uint8_t)(1u << 4u);
    wr8(m, bf_addr + 1u, v);

    /* ROM 0x1331BE: move.l #0x1331c6,(A0) */
    wr32(m, r->a[0], 0x001331C6u);
    /* ROM 0x1331C4: rts */
}

int Fn_1333C2(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x1333C2: subq.w #0x1,(0x6,A0) */
    uint16_t val = (uint16_t)rd16(m, r->a[0] + 6u);
    val--;
    wr16(m, r->a[0] + 6u, val);

    /* ROM 0x1333C6: bgt.b 0x001333ba */
    if ((int16_t)val > 0) {
        /* ROM 0x1333BA: move.l #0x1333c2,(A0) */
        wr32(m, r->a[0], 0x001333C2u);
        /* ROM 0x1333C0: rts -> returns to original caller */
        return 0;
    }

    /* ROM 0x1333C8: bra.w 0x001331ac */
    sub_1331AC(m, r);
    /* ROM 0x1331C4: rts -> returns to original caller */
    return 0;
}
