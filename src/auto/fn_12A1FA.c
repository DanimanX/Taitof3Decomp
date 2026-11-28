#include "fn_auto.h"
/* ROM 0x12A1FA: last state of the boss camera chain (A0 = object).
 * Ticks the camera via 0x11B288/0x11B2CA/0x11B262 (+0x06 timer, re-arming
 * 0x12A1FA while > 0); on expiry clears A5+0x30EE bits 1, 9 and 0 and falls into
 * the chain restart at 0x129D98 (target 0xDC/0x78, speed 2, A5+0x30F2 = 1,
 * 80-frame timer, arms 0x129DC0). */
#include "round8_camera_regs.h"

int Fn_12A1FA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a5 = r->a[5];

    if (tick_camera(m, r, 0x12A1FAu)) {
        wr32(m, obj, 0x12A1FAu);          /* 0x12A1F2 re-arm thunk */
        r->a[7] += 4u;                    /* rts */
        return 0;
    }

    wr8(m, a5 + 0x30EEu, (uint8_t)(rd8(m, a5 + 0x30EEu) & (uint8_t)~0x40u)); /* 0x12A212 bfclr {1:1} */
    wr8(m, a5 + 0x30EFu, (uint8_t)(rd8(m, a5 + 0x30EFu) & (uint8_t)~0x40u)); /* 0x12A218 bfclr {9:1} */
    wr8(m, a5 + 0x30EEu, (uint8_t)(rd8(m, a5 + 0x30EEu) & (uint8_t)~0x80u)); /* 0x12A21E bfclr {0:1} */

    /* 0x12A224 bra.w 0x129D98: restart block, runs into its own rts. */
    wr8(m, obj + 0x54u, 2u);              /* 0x129D98 */
    wr16(m, obj + 0x50u, 0x00DCu);        /* 0x129D9E */
    wr16(m, obj + 0x52u, 0x0078u);        /* 0x129DA4 */
    wr8(m, a5 + 0x30F2u, 1u);             /* 0x129DAA */
    wr16(m, obj + 6u, 0x0050u);           /* 0x129DB0 */
    wr32(m, obj, 0x129DC0u);              /* 0x129DB8 (BEQ 0x129DD8 never taken) */

    r->a[7] += 4u;                        /* 0x129DBE rts */
    return 0;
}
