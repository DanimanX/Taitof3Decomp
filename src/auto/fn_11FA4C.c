#include "fn_auto.h"
/* ROM 0x11FA4C: verified on a capture from the Y04 re-survey states: count down, then set the completion flag and install 0x11F7BE. */

int Fn_11FA4C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a5 = r->a[5];
    uint16_t timer = rd16(m, obj + 6u);
    wr16(m, obj + 6u, (uint16_t)(timer - 1u));

    /* Test BGT against the original timer to account for SUBQ overflow. */
    if ((int16_t)timer > 1) {
        wr32(m, obj, 0x11FA4Cu); /* 0x11FA44 */
    } else {
        wr8(m, a5 + 0x30EEu, rd8(m, a5 + 0x30EEu) | 0x40u); /* 0x11FA52 */
        wr32(m, obj, 0x11F7BEu); /* 0x11F7B6 */
    }
    r->a[7] += 4u;
    return 0;
}
