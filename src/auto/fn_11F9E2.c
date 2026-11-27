#include "fn_auto.h"
/* ROM 0x11F9E2: UNVERIFIED (no capture): count down, then clear the active flag, set the completion flag and install 0x11F7BE. */

int Fn_11F9E2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a5 = r->a[5];
    uint16_t timer = rd16(m, obj + 6u);
    wr16(m, obj + 6u, (uint16_t)(timer - 1u));

    /* Test BGT against the original timer to account for SUBQ overflow. */
    if ((int16_t)timer > 1) {
        wr32(m, obj, 0x11F9E2u); /* 0x11F9DA */
    } else {
        wr8(m, a5 + 0x30EFu, rd8(m, a5 + 0x30EFu) & 0x7Fu); /* 0x11F9E8 */
        wr8(m, a5 + 0x30EEu, rd8(m, a5 + 0x30EEu) | 0x40u); /* 0x11F9EE */
        wr32(m, obj, 0x11F7BEu); /* 0x11F7B6 */
    }
    r->a[7] += 4u;
    return 0;
}
