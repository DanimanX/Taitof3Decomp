#include "fn_auto.h"
/* ROM 0x118E48: ramp from the palette in D0 to the fixed black palette. */
#include "fn_palette_frames.h"

int Fn_118E48(const f3mem_t *m, f3regs_t *r)
{
    return call_palette_ramp(m, r, r->d[0], 0x00190Eu,
                             0x118E56u, 0x0018B0u, 0);
}
