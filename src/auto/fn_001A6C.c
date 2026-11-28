/* ROM 0x001A6C: IRQ5 increments the timer counter and returns through RTE. */
#include "../kernel/kernel.h"

int Fn_001A6C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t slot = r->a[5] - 0x5D52u;                        /* 0x001A6C */
    wr16(m, slot, (uint16_t)(rd16(m, slot) + 1u));            /* 0x001A6C addq.w #1 */
    Kern_RteFormat0(m, r);                                    /* 0x001A70 */
    return 0;
}
