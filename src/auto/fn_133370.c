/* ROM 0x133370: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_133370(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x133370: subq.w #1, 6(A0) */
    uint16_t cnt = rd16(m, r->a[0] + 6);
    cnt--;
    wr16(m, r->a[0] + 6, cnt);
    
    /* ROM 0x133374: bgt.b 0x133368 */
    if ((int16_t)cnt > 0) {
        /* ROM 0x133368: move.l #0x133370, (A0) */
        wr32(m, r->a[0], 0x133370u);
        
        /* ROM 0x13336E: rts */
        return 0;
    }

    /* ROM 0x133376: bfclr (0x30ee,A5){0x6:0x1} */
    uint8_t b = rd8(m, F3_A5 + 0x30EEu);
    b &= (uint8_t)~(1u << 6);
    wr8(m, F3_A5 + 0x30EEu, b);

    /* ROM 0x13337C: bfset (0x30ee,A5){0xb:0x1} */
    b = rd8(m, F3_A5 + 0x30EEu);
    b |= (uint8_t)(1u << 3);
    wr8(m, F3_A5 + 0x30EEu, b);

    /* ROM 0x133382: move.b #7, (0x30f2,A5) */
    wr8(m, F3_A5 + 0x30F2u, 7u);

    /* ROM 0x133388: bfset (0x30ee,A5){0xc:0x1} */
    b = rd8(m, F3_A5 + 0x30EEu);
    b |= (uint8_t)(1u << 4);
    wr8(m, F3_A5 + 0x30EEu, b);

    /* ROM 0x13338E: move.l #0x133396, (A0) */
    wr32(m, r->a[0], 0x133396u);

    /* ROM 0x133394: rts */
    return 0;
}
