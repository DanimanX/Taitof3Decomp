#include "fn_auto.h"
/* ROM 0x12A18E: boss camera tick; on expiry set A5+0x30EE bits 14, 1 and 0, clear
 * +0x54 and arm 0x12A1CC for 100 frames. */
#include "round8_camera_regs.h"

int Fn_12A18E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a5 = r->a[5];
    if (tick_camera(m, r, 0x12A18Eu)) {
        wr32(m, obj, 0x12A18Eu);          /* 0x12A186 re-arm thunk */
    } else {
        wr8(m, a5 + 0x30EFu, (uint8_t)(rd8(m, a5 + 0x30EFu) | 0x02u)); /* 0x12A1A6 bfset {14:1} */
        wr8(m, a5 + 0x30EEu, (uint8_t)(rd8(m, a5 + 0x30EEu) | 0x40u)); /* 0x12A1AC bfset {1:1} */
        wr8(m, a5 + 0x30EEu, (uint8_t)(rd8(m, a5 + 0x30EEu) | 0x80u)); /* 0x12A1B2 bfset {0:1} */
        wr8(m, obj + 0x54u, 0u);          /* 0x12A1B8 sf (0x54,A0) */
        wr16(m, obj + 6u, 0x0064u);       /* 0x12A1BC */
        wr32(m, obj, 0x12A1CCu);          /* 0x12A1C4 (BEQ 0x12A1E4 never taken) */
    }
    r->a[7] += 4u;                        /* rts */
    return 0;
}
