#include "fn_auto.h"

/* ROM 0x006020: the %p handler at 0x006004 has built eight lowercase hex
 * digits at A0. Set their length and the optional %#p prefix before the
 * formatter enters the shared render code at 0x0062E8. */
uint16_t Formatter_Prepare_006020(f3regs_t *r, uint16_t flags)
{
    r->d[1] = 0;
    if (flags & 0x40u) {
        r->d[1] = 2;
        r->a[1] = 0x005F9Eu; /* "0x" */
    }
    return 8;
}

/* Mid-function entry with the active formatter frame and registers. After
 * rendering, control returns to the format loop at 0x005C76. */
int Fn_006020(const f3mem_t *m, f3regs_t *r)
{
    uint16_t length = Formatter_Prepare_006020(r, (uint16_t)r->d[3]);
    return Formatter_RenderAndResume(m, r, length);
}
