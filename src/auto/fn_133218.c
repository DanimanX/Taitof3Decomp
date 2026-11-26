/* ROM 0x133218: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_133218(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x133218: subq.w #1, (0x6, A0) */
    uint16_t tmp = rd16(m, r->a[0] + 6);
    tmp--;
    wr16(m, r->a[0] + 6, tmp);
    
    /* ROM 0x13321C: bgt.b 0x133210 */
    if (tmp > 0) {
        /* ROM 0x133210: move.l #0x133218, (A0) */
        wr32(m, r->a[0], 0x133218);
        return 0;
    }

    /* ROM 0x13321E: bfset (0x30ee,A5){0x9:0x1} -> bit 6 (0x40) */
    uint8_t v = rd8(m, F3_A5 + 0x30EE);
    v |= 0x40;
    wr8(m, F3_A5 + 0x30EE, v);

    /* ROM 0x133224: move.b #0, (0x30f2,A5) */
    wr8(m, F3_A5 + 0x30F2, 0);

    /* ROM 0x13322A: bfset (0x30ee,A5){0xc:0x1} -> bit 3 (0x08) */
    v = rd8(m, F3_A5 + 0x30EE);
    v |= 0x08;
    wr8(m, F3_A5 + 0x30EE, v);

    /* ROM 0x133230: move.w #0x168, (0x6, A0) */
    wr16(m, r->a[0] + 6, 0x168);

    /* ROM 0x133236: beq.b 0x133246 (never taken, Z=0) -> falls through */
    /* ROM 0x133238: move.l #0x133240, (A0) */
    wr32(m, r->a[0], 0x133240);
    return 0;
}
