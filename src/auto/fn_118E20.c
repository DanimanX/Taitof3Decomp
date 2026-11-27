#include "fn_auto.h"
/* ROM 0x118E20: ramp from the palette in D0 to the fixed white palette. */
#include "fn_palette_frames.h"

int Fn_118E20(const f3mem_t *m, f3regs_t *r)
{
    return call_palette_ramp(m, r, r->d[0], 0x00198Eu,
                             0x118E2Eu, 0x001904u, 0);
}
