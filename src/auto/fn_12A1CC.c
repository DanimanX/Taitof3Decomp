#include "fn_auto.h"
/* ROM 0x12A1CC: boss camera tick; on expiry set A5+0x30EE bit 9, load a 300-frame
 * timer and arm 0x12A1FA. */
#include "round8_camera_regs.h"

int Fn_12A1CC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (tick_camera(m, r, 0x12A1CCu)) {
        wr32(m, obj, 0x12A1CCu);          /* 0x12A1C4 re-arm thunk */
    } else {
        /* 0x12A1E4 bfset (0x30EE,A5){9:1}: byte A5+0x30EF bit 6. */
        wr8(m, r->a[5] + 0x30EFu,
            (uint8_t)(rd8(m, r->a[5] + 0x30EFu) | 0x40u));
        wr16(m, obj + 6u, 0x012Cu);       /* 0x12A1EA */
        wr32(m, obj, 0x12A1FAu);          /* 0x12A1F2 (BEQ 0x12A212 never taken) */
    }
    r->a[7] += 4u;                        /* rts */
    return 0;
}
