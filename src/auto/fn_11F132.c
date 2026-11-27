#include "fn_auto.h"
/* ROM 0x11F132: verified on a capture from the Y04 re-survey states: Jitter the stage coordinate while counting down, then initialize the object's movement. */

int Fn_11F132(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t globals = r->a[5];
    uint32_t x = globals + 0x30D2u;
    if ((rd8(m, globals + 0x1D58u) & 0x02u) != 0u) {
        wr32(m, r->a[7] - 4u, 0x11F140u);
        uint32_t rng = Rng_NextMem(m); /* 0x11F13A */
        uint16_t delta = (uint16_t)((rng & 3u) - 1u);
        r->d[0] = (rng & 0xFFFF0000u) | delta;
        uint16_t high = rd16(m, x);
        if ((int16_t)high > 0xA0)
            wr16(m, x, (uint16_t)(high - delta)); /* 0x11F14E */
        else
            wr16(m, x, (uint16_t)(high + delta)); /* 0x11F154 */
    }

    uint16_t count = rd16(m, obj + 6u);
    wr16(m, obj + 6u, (uint16_t)(count - 1u)); /* 0x11F158 */
    if ((int16_t)count > 1) {
        wr32(m, obj, 0x11F132u); /* 0x11F12A */
        return 0;
    }

    /* 0x11F168 extension 0x0800 selects signed 32/32, with D0 as both
     * destination fields. The quotient is the result left in D0. */
    int32_t dividend = (int32_t)(rd32(m, x) - 0x4C0000u);
    r->d[0] = (uint32_t)(dividend / 0x62);
    wr32(m, obj + 0x14u, r->d[0]);                     /* 0x11F170 */
    wr32(m, obj + 0x18u, rd32(m, globals + 0x30D6u)); /* 0x11F174 */
    wr16(m, obj + 0x20u, 0u);                          /* 0x11F17A */
    wr16(m, obj + 0x42u, 0x80u);                       /* 0x11F180 */
    wr32(m, obj, 0x11F18Eu);                           /* 0x11F186 */
    return 0;
}
