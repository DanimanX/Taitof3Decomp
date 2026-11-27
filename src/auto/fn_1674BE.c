/* ROM 0x1674BE: sets +0x44.w = 0x0080 and arms 0x1674CC. */
#include "fn_auto.h"

int Fn_1674BE(const f3mem_t *m, f3regs_t *r)
{
    wr16(m, r->a[0] + 0x44u, 0x0080u);                  /* 0x1674BE */
    wr32(m, r->a[0], 0x001674CCu);                      /* 0x1674C4 */
    return 0;
}
