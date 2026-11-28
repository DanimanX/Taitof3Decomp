/* ROM 0x001C2C: initialize the scroll/tile base table at 0x402242.
 * Writes ten words depending on the flag word at A5-0x5D58 (0x4022A8):
 * zero -> the 0x0..0x29/0x18 layout, non-zero -> the 0x2600..0x100 layout.
 * A0 ends 0x14 past 0x402242 on both paths; no other register changes. */
#include "fn_auto.h"

int Fn_001C2C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = 0x00402242u;                       /* 0x001C2C lea */

    if (rd16(m, F3_A5 - 0x5D58u) == 0u) {            /* 0x001C32 tst.w / bne 0x1C66 */
        wr16(m, a0, 0xF600u);                        /* 0x001C3A move.w #-0xA00 */
        a0 += 2u;
        wr16(m, a0, 0xFF80u);                        /* 0x001C3E */
        a0 += 2u;
        wr16(m, a0, 0xF700u);                        /* 0x001C42 */
        a0 += 2u;
        wr16(m, a0, 0xF400u);                        /* 0x001C46 */
        a0 += 2u;
        wr16(m, a0, 0xF800u);                        /* 0x001C4A */
        a0 += 2u;
        wr16(m, a0, 0xF400u);                        /* 0x001C4E */
        a0 += 2u;
        wr16(m, a0, 0xF900u);                        /* 0x001C52 */
        a0 += 2u;
        wr16(m, a0, 0xFF80u);                        /* 0x001C56 */
        a0 += 2u;
        wr16(m, a0, 0x0029u);                        /* 0x001C5A */
        a0 += 2u;
        wr16(m, a0, 0x0018u);                        /* 0x001C5E */
        a0 += 2u;                                    /* 0x001C62 bra 0x1C8E */
    } else {
        wr16(m, a0, 0x2600u);                        /* 0x001C66 */
        a0 += 2u;
        wr16(m, a0, 0x8B80u);                        /* 0x001C6A #-0x7480 */
        a0 += 2u;
        wr16(m, a0, 0x2700u);                        /* 0x001C6E */
        a0 += 2u;
        wr16(m, a0, 0x8000u);                        /* 0x001C72 #-0x8000 */
        a0 += 2u;
        wr16(m, a0, 0x2800u);                        /* 0x001C76 */
        a0 += 2u;
        wr16(m, a0, 0x8000u);                        /* 0x001C7A */
        a0 += 2u;
        wr16(m, a0, 0x2900u);                        /* 0x001C7E */
        a0 += 2u;
        wr16(m, a0, 0x8B80u);                        /* 0x001C82 */
        a0 += 2u;
        wr16(m, a0, 0x009Eu);                        /* 0x001C86 */
        a0 += 2u;
        wr16(m, a0, 0x0100u);                        /* 0x001C8A */
        a0 += 2u;
    }                                                /* 0x001C8E rts */

    r->a[0] = a0;
    return 0;
}
