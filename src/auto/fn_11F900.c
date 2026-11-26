/* ROM 0x11F900: countdown state transition tick (A0 = object).
 * Decrements word +0x06; while > 0 re-arms 0x11F900. Otherwise if A5+0x30F4
 * is zero sets +0x06 = 0xA and transitions to 0x11F91E; if non-zero sets
 * A5+0x3262 = 0xFF, sets A5+0x30EE bit 8 (0x30EF |= 0x80), sets +0x06 = 0x12C,
 * and transitions to 0x11F9E2. */
#include "fn_auto.h"

int Fn_11F900(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x11F900 */
    wr16(m, obj + 0x06u, (uint16_t)cnt);
    if (cnt > 0) {                                        /* 0x11F904 */
        wr32(m, obj, 0x0011F900u);                        /* 0x11F8F8 */
        return 0;
    }

    if (rd8(m, F3_A5 + 0x30F4u) == 0u) {                  /* 0x11F906..0x11F90A */
        wr16(m, obj + 0x06u, 0x000Au);                    /* 0x11F90E (beq 0x11F924 never taken) */
        wr32(m, obj, 0x0011F91Eu);                        /* 0x11F916 */
        return 0;
    }

    wr8(m, F3_A5 + 0x3262u, 0u);                          /* 0x11F9C8: sf */
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x80u)); /* 0x11F9CC: bfset {8:1} */
    wr16(m, obj + 0x06u, 0x012Cu);                        /* 0x11F9D2 (beq 0x11F9E8 never taken) */
    wr32(m, obj, 0x0011F9E2u);                            /* 0x11F9DA */
    return 0;
}
