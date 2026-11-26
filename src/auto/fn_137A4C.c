/* ROM 0x137A4C: boss scroll-offset step & sound trigger tick (A0 = object).
 * Sign-extends byte +0x42 into D0, subtracts 0x40 from word +0x42, adds D0.w
 * to A5+0x32D4 and subtracts it from A5+0x32D0. While (int16_t)(A5+0x32D0) < 0
 * re-arms 0x137A4C; otherwise clears A5+0x32D0/0x32D4, plays Sound_PlayHooked
 * 0x3A, sets +0x06 = 0x3C, and transitions to 0x137A92. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

int Fn_137A4C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* 0x137A4C: bfexts (0x42,A0){0x0:0x8},D0 */
    int32_t d0 = (int32_t)(int8_t)rd8(m, obj + 0x42u);
    r->d[0] = (uint32_t)d0;

    /* 0x137A52..0x137A64 */
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) - 0x0040u));
    wr16(m, F3_A5 + 0x32D4u, (uint16_t)(rd16(m, F3_A5 + 0x32D4u) + (uint16_t)d0));
    int16_t v = (int16_t)(rd16(m, F3_A5 + 0x32D0u) - (int16_t)d0);
    wr16(m, F3_A5 + 0x32D0u, (uint16_t)v);
    if (v < 0) {
        wr32(m, obj, 0x00137A4Cu);                       /* 0x137A44 */
        return 0;
    }

    /* 0x137A66..0x137A90 */
    wr16(m, F3_A5 + 0x32D0u, 0u);
    wr16(m, F3_A5 + 0x32D4u, 0u);
    Sound_PlayHooked(m, 0x003Au);
    wr16(m, obj + 0x06u, 0x003Cu);                        /* beq 0x137A98 never taken */
    wr32(m, obj, 0x00137A92u);                            /* 0x137A8A */
    return 0;
}
