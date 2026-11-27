#include "fn_auto.h"
/* ROM 0x118E72: ramp from the palette in D4 to the palette in D0. */
#include "fn_palette_frames.h"

int Fn_118E72(const f3mem_t *m, f3regs_t *r)
{
    return call_palette_ramp(m, r, r->d[4], r->d[0],
                             0x118E82u, 0x00185Cu, 1);
}
