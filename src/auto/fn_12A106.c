#include "fn_auto.h"
/* ROM 0x12A106: boss camera tick; on expiry clear A5+0x30EE bits 0-1 and bit 3,
 * load a 60-frame timer and arm 0x12A13A. */
#include "round8_camera_regs.h"

int Fn_12A106(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (tick_camera(m, r, 0x12A106u)) {
        wr32(m, obj, 0x12A106u);          /* 0x12A0FE re-arm thunk */
    } else {
        /* 0x12A11E bfclr (0x30EE,A5){0:2}: byte A5+0x30EE bits 7..6. */
        wr8(m, r->a[5] + 0x30EEu,
            (uint8_t)(rd8(m, r->a[5] + 0x30EEu) & (uint8_t)~0xC0u));
        /* 0x12A124 bfclr (0x30EE,A5){3:1}: same byte, bit 4. */
        wr8(m, r->a[5] + 0x30EEu,
            (uint8_t)(rd8(m, r->a[5] + 0x30EEu) & (uint8_t)~0x10u));
        wr16(m, obj + 6u, 0x003Cu);       /* 0x12A12A */
        wr32(m, obj, 0x12A13Au);          /* 0x12A132 (BEQ 0x12A152 never taken) */
    }
    r->a[7] += 4u;                        /* rts */
    return 0;
}
