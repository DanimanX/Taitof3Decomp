#include "fn_auto.h"
/* ROM 0x11FA14: verified on a capture from the Y04 re-survey states: Count down before setting a stage flag and starting a new timer. */

int Fn_11FA14(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t count = rd16(m, obj + 6u);
    wr16(m, obj + 6u, (uint16_t)(count - 1u)); /* 0x11FA14 */
    if ((int16_t)count > 1) {
        wr32(m, obj, 0x11FA14u); /* 0x11FA0C */
        return 0;
    }

    /* 0x11FA1A BFSET {9:1} addresses bit 6 of the second byte. */
    uint32_t flags = r->a[5] + 0x30EFu;
    wr8(m, flags, (uint8_t)(rd8(m, flags) | 0x40u));
    /* 0x11FA20 clears Z, so 0x11FA26 falls through to 0x11FA28. */
    wr16(m, obj + 6u, 0xB4u);
    wr32(m, obj, 0x11FA30u);  /* 0x11FA28 */
    return 0;
}
