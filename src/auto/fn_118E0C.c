#include "fn_auto.h"
/* ROM 0x118E0C: ramp from the fixed black palette to the palette in D0. */
#include "fn_palette_frames.h"

int Fn_118E0C(const f3mem_t *m, f3regs_t *r)
{
    return call_palette_ramp(m, r, 0x00190Eu, r->d[0],
                             0x118E1Au, 0x001886u, 0);
}
