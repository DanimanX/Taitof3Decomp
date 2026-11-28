/* ROM 0x001034: (recipe capture fnk_, work/zcap/recipes15.tsv): one-frame countdown of the word at
 * A5-0x7F70 (0x400090). The whole routine is `subq.w #1,-0x7f70(a5) ; rts`
 * (bytes 53 6D 80 90 / 4E 75); the code from 0x00103A on is the next routine. */
#include "fn_auto.h"

int Fn_001034(const f3mem_t *m, f3regs_t *r)
{
    uint32_t slot = r->a[5] - 0x7F70u;                        /* 0x001034 */
    wr16(m, slot, (uint16_t)(rd16(m, slot) - 1u));            /* 0x001034 subq.w #1 */
    return 0;                                                 /* 0x001038 rts */
}
