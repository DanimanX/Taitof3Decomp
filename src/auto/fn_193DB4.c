#include "fn_auto.h"
/* ROM 0x193DB4: checked against the ROM by Claude; no capture can exercise it (its writes never change memory): Re-arm the current object callback. */

int Fn_193DB4(const f3mem_t *m, f3regs_t *r)
{
    /* 0x193DB4 branches back to the store and RTS at 0x193DAC. */
    wr32(m, r->a[0], 0x193DB4u);
    return 0;
}
