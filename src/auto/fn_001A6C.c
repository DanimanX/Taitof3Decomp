/* ROM 0x001A6C: UNVERIFIED (no capture): IRQ5 (timer) handler.
 * `addq.w #1,-0x5d52(a5) ; rte` (bytes 52 6D A2 AE / 4E 73): it only bumps the
 * IRQ5 counter at A5-0x5D52 (0x4022AE) and returns through the exception frame.
 * Like the Kern_* handlers the RTE is not executed (SR/PC live outside
 * f3regs_t), so A7 is left pointing at the SR/PC frame and the entry returns
 * the PC of the RTE, 0x001A70. */
#include "fn_auto.h"

int Fn_001A6C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t slot = r->a[5] - 0x5D52u;                        /* 0x001A6C */
    wr16(m, slot, (uint16_t)(rd16(m, slot) + 1u));            /* 0x001A6C addq.w #1 */
    return 0;                                                 /* 0x001A70 rte */
}
