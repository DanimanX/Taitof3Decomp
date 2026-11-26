/* ROM 0x13311A: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_13311A(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x13311A: subq.w #0x1, (0x6, A0) */
    uint16_t cnt = rd16(m, (uint32_t)r->a[0] + 6u);
    cnt--;
    wr16(m, (uint32_t)r->a[0] + 6u, cnt);
    if (cnt > 0) {
        /* ROM 0x133112: move.l #0x13311a, (A0) ; rts */
        wr32(m, (uint32_t)r->a[0], 0x13311Au);
        return 0;
    }

    /* ROM 0x133120: movea.l (0x1d28, A5), A1 */
    r->a[1] = rd32(m, F3_A5 + 0x1D28u);
    /* ROM 0x133124: move.w (0x14, A1), (0x30d2, A5) */
    wr16(m, F3_A5 + 0x30D2u, rd16(m, (uint32_t)r->a[1] + 0x14u));
    /* ROM 0x13312A: addi.w #0x14, (0x30d2, A5) */
    uint16_t tmp = rd16(m, F3_A5 + 0x30D2u);
    tmp += 0x14u;
    wr16(m, F3_A5 + 0x30D2u, tmp);
    /* ROM 0x133130: move.b #0x3, (0x30f2, A5) */
    wr8(m, F3_A5 + 0x30F2u, 3u);
    /* ROM 0x133136: bfset (0x30ee, A5){0xc:0x1} */
    tmp = rd8(m, F3_A5 + 0x30EEu);
    tmp |= (1u << (0xC & 7));
    wr8(m, F3_A5 + 0x30EEu, tmp);
    /* ROM 0x13313C: move.w #0xb4, (0x6, A0) */
    wr16(m, (uint32_t)r->a[0] + 6u, 0xB4u);
    /* ROM 0x133142: beq.b 0x133152 (never taken, folded) */
    /* ROM 0x133144: move.l #0x13314c, (A0) ; rts */
    wr32(m, (uint32_t)r->a[0], 0x13314Cu);
    return 0;
}
