/* ROM 0x11CDD8: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

static void thunk_11CDD0(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x11CDD0: move.l #0x11cdd8,(A0) */
    wr32(m, r->a[0], 0x0011CDD8u);
    /* ROM 0x11CDD6: rts */
}

static void routine_11CCEE(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x11CCEE: move.w (0x3a,A0),(0x20,A0) */
    uint16_t v = rd16(m, r->a[0] + 0x3Au);
    wr16(m, r->a[0] + 0x20u, v);
    /* ROM 0x11CCF4: move.w #0xf0,(0x6,A0) */
    wr16(m, r->a[0] + 0x6u, 0x00F0u);
    /* ROM 0x11CCFA: bfset (0x30ee,A5){0x5:0x1} */
    uint8_t b = rd8(m, F3_A5 + 0x30EEu);
    b |= 0x04u;
    wr8(m, F3_A5 + 0x30EEu, b);
    /* ROM 0x11CD00: move.w #0x6,(0x42,A0) */
    wr16(m, r->a[0] + 0x42u, 0x0006u);
    /* ROM 0x11CD06: move.l #0x11cd0e,(A0) */
    wr32(m, r->a[0], 0x0011CD0Eu);
    /* ROM 0x11CD0C: rts */
}

int Fn_11CDD8(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x11CDD8: subq.w #0x1,(0x6,A0) */
    uint16_t cnt = rd16(m, r->a[0] + 0x6u);
    cnt--;
    wr16(m, r->a[0] + 0x6u, cnt);
    /* ROM 0x11CDDC: bgt.b 0x0011cdd0 */
    if ((int16_t)cnt > 0) {
        thunk_11CDD0(m, r);
        return 0;
    }
    /* ROM 0x11CDDE: bfset (0x30ee,A5){0x2:0x1} */
    uint8_t b = rd8(m, F3_A5 + 0x30EEu);
    b |= 0x20u;
    wr8(m, F3_A5 + 0x30EEu, b);
    /* ROM 0x11CDE4: bra.w 0x0011ccee */
    routine_11CCEE(m, r);
    return 0;
}
