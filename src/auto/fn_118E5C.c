#include "fn_auto.h"
/* ROM 0x118E5C: ramp from the palette in D0 to the palette in D4. */
#include "fn_palette_frames.h"

int Fn_118E5C(const f3mem_t *m, f3regs_t *r)
{
    return call_palette_ramp(m, r, r->d[0], r->d[4],
                             0x118E6Cu, 0x00185Cu, 1);
}
