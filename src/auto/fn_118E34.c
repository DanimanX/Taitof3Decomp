#include "fn_auto.h"
/* ROM 0x118E34: ramp from the fixed white palette to the palette in D0. */
#include "fn_palette_frames.h"

int Fn_118E34(const f3mem_t *m, f3regs_t *r)
{
    return call_palette_ramp(m, r, 0x00198Eu, r->d[0],
                             0x118E42u, 0x0018DAu, 0);
}
