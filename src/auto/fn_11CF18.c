#include "fn_auto.h"
/* ROM 0x11CF18: count down the boss state, then set its flag and advance the script. */

int Fn_11CF18(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t timer = rd16(m, obj + 6u);
    wr16(m, obj + 6u, (uint16_t)(timer - 1u));            /* 0x11CF18 SUBQ.W */
    if ((int16_t)timer > 1) {                             /* 0x11CF1C BGT includes V */
        wr32(m, obj, 0x11CF18u);                          /* 0x11CF10 */
    } else {
        uint32_t flags = r->a[5] + 0x30EEu;
        wr8(m, flags, (uint8_t)(rd8(m, flags) | 0x10u));  /* 0x11CF1E BFSET {3:1} */
        wr16(m, obj + 6u, 0x008Cu);                       /* BEQ at 0x11CF2A is dead */
        wr32(m, obj, 0x11CF34u);                          /* 0x11CF2C */
    }
    r->a[7] += 4u;
    return 0;
}
