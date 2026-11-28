/* ROM 0x0054E8: UNVERIFIED (no capture): run-length tile-row unpacker.
 * A0 points at {count.b, count.b, ops...}; the two counts are stored as words
 * into the scratch buffer A5-0x12F0 (0x406D10) and their product is the total
 * number of 32-bit cells to write. Each op is {kind.b, run.b, ...} dispatched
 * through the 5-entry table at 0x5516 (base = 0x005504 + 0x12 = 0x5516,
 * entries 0x552A / 0x5534 / 0x5542 / 0x5552 / 0x5564):
 *   0: copy one long (the run byte is unused);
 *   1: run+2 zero longs;
 *   2: run+2 copies of the next long;
 *   3: run+2 longs ascending from the next long (+1 per cell, low word only);
 *   4: run+2 longs descending from the next long (-1 per cell, low word only).
 * (The run byte n lands in D1; the handler writes once, then `dbra D1` runs the
 * body n+1 more times, so the count is n+2.)
 * Every written long decrements the cell counter (low word, signed compare) and
 * the routine stops as soon as it reaches 0 or below (the rest of a run is
 * dropped), returning the buffer base in A0. An op byte above 4 indexes past
 * the 5-entry table into code, i.e. the ROM never does it; the model stops.
 * The movem pair restores D0-D2/A1/A2, so only A0 and RAM change. */
#include "fn_auto.h"

int Fn_0054E8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    uint32_t a1 = r->a[5] - 0x12F0u;                  /* 0x0054EC lea -0x12F0(A5),A1 */
    uint16_t d0w, d1w;

    d0w = (uint16_t)rd8(m, a0); a0 += 1u;             /* 0x0054F0 moveq #0,D0 ; move.b (A0)+,D0 */
    wr16(m, a1, d0w); a1 += 2u;                       /* 0x0054F4 move.w D0,(A1)+ */
    d1w = (uint16_t)rd8(m, a0); a0 += 1u;             /* 0x0054F6 moveq #0,D1 ; move.b (A0)+,D1 */
    wr16(m, a1, d1w); a1 += 2u;                       /* 0x0054FA move.w D1,(A1)+ */
    d1w = (uint16_t)((int16_t)d0w * (int16_t)d1w);    /* 0x0054FC muls.w D1,D0 */

    for (;;) {
        uint16_t runs;
        uint32_t val = 0;
        int adjust = 0;
        uint8_t op;

        op = rd8(m, a0); a0 += 1u;                    /* 0x0054FE moveq #0,D1 ; move.b (A0)+,D1 */
        if (op > 4u)
            break;                                    /* past the 0x5516 table: no handler */
        runs = (uint16_t)((uint16_t)rd8(m, a0) + 2u); /* 0x005502 run byte */
        a0 += 1u;
        switch (op) {
        case 0:                                       /* 0x00552A */
            runs = 1u;
            break;
        case 1:                                       /* 0x005534 */
            break;
        case 2:                                       /* 0x005542 */
            val = rd32(m, a0); a0 += 4u;              /* 0x005544 move.l (A0)+,D2 */
            break;
        case 3:                                       /* 0x005552 */
            val = rd32(m, a0); a0 += 4u;              /* 0x005554 */
            adjust = 1;
            break;
        default:                                      /* 0x005564 */
            val = rd32(m, a0); a0 += 4u;              /* 0x005566 */
            adjust = -1;
            break;
        }

        for (uint16_t i = 0; i < runs; i++) {
            if (op == 0u) {
                val = rd32(m, a0); a0 += 4u;          /* 0x00552C move.l (A0)+,(A1)+ */
            } else if (op == 1u) {
                val = 0u;                             /* 0x005536 clr.l (A1)+ */
            }
            wr32(m, a1, val); a1 += 4u;
            if (adjust > 0)                           /* 0x005558 addq.w #1,D2 */
                val = (val & 0xFFFF0000u) | (uint16_t)(val + 1u);
            else if (adjust < 0)                      /* 0x00556A subq.w #1,D2 */
                val = (val & 0xFFFF0000u) | (uint16_t)(val - 1u);
            d1w = (uint16_t)(d1w - 1u);               /* 0x00552E subq.w #1,D0 */
            if ((int16_t)d1w <= 0)                    /* 0x005530 ble 0x00550C */
                goto done;
        }
    }
done:
    r->a[0] = r->a[5] - 0x12F0u;                      /* 0x005510 lea -0x12F0(A5),A0 ; rts */
    return 0;
}
