#include "fn_auto.h"
/* ROM 0x001608: select a timed palette script when its duration is positive. */
#include "fn_call_frames.h"

int Fn_001608(const f3mem_t *m, f3regs_t *r)
{
    uint32_t entry = r->a[0] + 8 * (int16_t)r->d[0];
    r->d[1] = rd32(m, entry + 4);
    if ((int32_t)r->d[1] > 0) {
        r->d[1] = set_word(r->d[1], (uint16_t)(r->d[1] + 1u));
        wr16(m, r->a[5] - 0x5DE8u, (uint16_t)r->d[1]);
        wr32(m, r->a[5] - 0x5DECu, rd32(m, entry));
    }
    r->a[7] += 4;
    return 0;
}
