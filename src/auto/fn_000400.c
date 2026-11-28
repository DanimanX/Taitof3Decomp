/* ROM 0x000400: UNVERIFIED (no capture): spurious-interrupt exception vector.
 * The vector at 0x000400 holds a single instruction, `rte` (bytes 4E 73); the
 * following code (0x000402 lea 0x620000,...) is the next routine. Nothing is
 * read or written: the handler just abandons the spurious interrupt frame.
 * The RTE is not executed (SR/PC live outside f3regs_t), so A7 is left pointing
 * at the 6-byte SR/PC frame and the entry returns the RTE PC, 0x000400. */
#include "fn_auto.h"

int Fn_000400(const f3mem_t *m, f3regs_t *r)
{
    (void)m;
    (void)r;
    return 0;                                    /* 0x000400 rte */
}
