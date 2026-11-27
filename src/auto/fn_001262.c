#include "fn_auto.h"
/* ROM 0x001262: copy the sprite control word to both sprite RAM banks. */
#include "fn_call_frames.h"

int Fn_001262(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, r->a[0]);
    r->a[0] = 0x600000u;
    wr16(m, r->a[0] + 0xAu, rd16(m, r->a[5] - 0xCA4u));
    wr16(m, r->a[0] + 0x800Au, rd16(m, r->a[5] - 0xCA4u));
    r->a[0] = rd32(m, r->a[7]);
    r->a[7] += 8;
    return 0;
}
