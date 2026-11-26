/* ROM 0x133178: boss phase countdown tick (A0 = object).
 * Decrements +0x06 and re-arms 0x133178 while > 0. Once <= 0, clears
 * A5+0x30F2, sets A5+0x30EE bit 12 (0x30EF |= 0x08), and checks A5+0x30DA:
 * if > 0x320, sets +0x06 = 0x3C and transitions to 0x13305A; otherwise plays
 * sound 0x13 via Sound_PlayHooked, sets A5+0x30EE bits 11 and 12, sets
 * A5+0x30F2 = 7, and transitions to 0x1331C6. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

int Fn_133178(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x133178 */
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {                                      /* 0x13317C */
        wr32(m, obj, 0x00133178u);                    /* 0x133170 */
        return 0;
    }

    wr8(m, F3_A5 + 0x30F2u, 0u);                      /* 0x13317E */
    /* 0x133184: bfset (0x30ee,A5){0xc:1} */
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x08u));

    if ((int16_t)rd16(m, F3_A5 + 0x30DAu) > 0x0320) { /* 0x13318A..0x133194 */
        wr16(m, obj + 0x06u, 0x003Cu);                /* 0x13304C */
        wr32(m, obj, 0x0013305Au);                    /* 0x133052 */
        return 0;
    }

    Sound_PlayHooked(m, 0x0013u);                     /* 0x133198..0x1331A8 */
    /* 0x1331AC: bfset (0x30ee,A5){0xb:1} */
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x10u));
    wr8(m, F3_A5 + 0x30F2u, 7u);                      /* 0x1331B2 */
    /* 0x1331B8: bfset (0x30ee,A5){0xc:1} */
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x08u));
    wr32(m, obj, 0x001331C6u);                        /* 0x1331BE */
    return 0;
}
