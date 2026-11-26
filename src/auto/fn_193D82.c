/* ROM 0x193D82: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_193D82(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x193D82: movea.l (0x8,A0),A4 */
    r->a[4] = rd32(m, r->a[0] + 8u);
    /* ROM 0x193D86: lea (0x10,A0),A1 */
    r->a[1] = r->a[0] + 16u;
    /* ROM 0x193D8A: move.l A1,(0x68,A4) */
    wr32(m, r->a[4] + 0x68u, r->a[1]);
    /* ROM 0x193D8E: clr.w (0x62,A4) */
    wr16(m, r->a[4] + 0x62u, 0u);
    /* ROM 0x193D92: move.w #0x7,D0w */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | 7u;

    /* ROM 0x193D96: loop start */
    for (;;) {
        /* ROM 0x193D96: move.w #-0x64,(A1)+ */
        wr16(m, r->a[1], (uint16_t)0xFF9Cu);
        r->a[1] += 2u;
        /* ROM 0x193D9A: move.w #-0x64,(A1)+ */
        wr16(m, r->a[1], (uint16_t)0xFF9Cu);
        r->a[1] += 2u;
        /* ROM 0x193D9E: move.w #0x0,(A1)+ */
        wr16(m, r->a[1], 0u);
        r->a[1] += 2u;
        /* ROM 0x193DA2: move.l #0x28740,(A1)+ */
        wr32(m, r->a[1], 0x28740u);
        r->a[1] += 4u;
        /* ROM 0x193DA8: dbf D0w,0x00193d96 */
        uint16_t d0w = (uint16_t)r->d[0];
        d0w--;
        r->d[0] = (r->d[0] & 0xFFFF0000u) | d0w;
        if (d0w != 0xFFFFu) {
            continue;
        }
        break;
    }

    /* ROM 0x193DAC: move.l #0x193db4,(A0) */
    wr32(m, r->a[0], 0x193DB4u);
    /* ROM 0x193DB2: rts */
    return 0;
}
