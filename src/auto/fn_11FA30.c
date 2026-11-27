#include "fn_auto.h"
/* ROM 0x11FA30: verified on a capture from the Y04 re-survey states: count down before clearing a flag and starting the 30-frame 0x11FA4C state. */

int Fn_11FA30(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a5 = r->a[5];
    uint16_t timer = rd16(m, obj + 6u);
    wr16(m, obj + 6u, (uint16_t)(timer - 1u));

    /* Test BGT against the original timer to account for SUBQ overflow. */
    if ((int16_t)timer > 1) {
        wr32(m, obj, 0x11FA30u); /* 0x11FA28 */
    } else {
        wr8(m, a5 + 0x30EFu, rd8(m, a5 + 0x30EFu) & 0xBFu); /* 0x11FA36 */
        wr16(m, obj + 6u, 0x1Eu); /* 0x11FA3C sets Z=0, so 0x11FA42 falls through. */
        wr32(m, obj, 0x11FA4Cu); /* 0x11FA44 */
    }
    r->a[7] += 4u;
    return 0;
}
