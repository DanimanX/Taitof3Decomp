/* ROM 0x11F94C: boss glow/shutter oscillation tick (A0 = object).
 * Every 8 ticks (+0x3E), oscillates A5+0x3262 between 2 and 4 using +0x3A
 * direction flag. Counts down +0x06 and re-arms 0x11F94C while > 0; when <= 0,
 * clears A5+0x30EE bit 0 (mask 0x01), decrements A5+0x3262, sets +0x06 = 10,
 * and transitions to 0x11F9AA. */
#include "fn_auto.h"

int Fn_11F94C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint16_t w3e = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u); /* 0x11F94C */
    wr16(m, obj + 0x3Eu, w3e);
    if ((int16_t)w3e >= 8) {                            /* 0x11F950 */
        wr16(m, obj + 0x3Eu, 0u);                       /* 0x11F95A */
        if (rd8(m, obj + 0x3Au) == 0u) {                /* 0x11F95E */
            uint8_t b = (uint8_t)(rd8(m, F3_A5 + 0x3262u) - 1u); /* 0x11F966 */
            wr8(m, F3_A5 + 0x3262u, b);
            if ((int8_t)b <= 2)                         /* 0x11F96A */
                wr8(m, obj + 0x3Au, (uint8_t)~rd8(m, obj + 0x3Au)); /* 0x11F986 */
        } else {
            uint8_t b = (uint8_t)(rd8(m, F3_A5 + 0x3262u) + 1u); /* 0x11F978 */
            wr8(m, F3_A5 + 0x3262u, b);
            if ((int8_t)b >= 4)                         /* 0x11F97C */
                wr8(m, obj + 0x3Au, (uint8_t)~rd8(m, obj + 0x3Au)); /* 0x11F986 */
        }
    }

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x11F98A */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                             /* 0x11F98E */
        wr32(m, obj, 0x0011F94Cu);                      /* 0x11F944 */
        return 0;
    }

    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & ~0x01u)); /* 0x11F990 */
    wr8(m, F3_A5 + 0x3262u, (uint8_t)(rd8(m, F3_A5 + 0x3262u) - 1u)); /* 0x11F996 */
    wr16(m, obj + 0x06u, 0x000Au);                      /* 0x11F99A */
    wr32(m, obj, 0x0011F9AAu);                          /* 0x11F9A2 */
    return 0;
}
