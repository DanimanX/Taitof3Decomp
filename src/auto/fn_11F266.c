#include "fn_auto.h"

/* ROM 0x11F266: verified on a capture from the Y04 re-survey states: jitter the shared X position and transition when the timer expires. */

/* ROM 0x11EEE0: set the exit flags and choose the next horizontal motion. */
static void finish_wait(const f3mem_t *m, uint32_t obj, uint32_t a5)
{
    uint32_t flags = a5 + 0x30EEu;
    /* Bitfield offsets 12 and 13 are bits 3 and 2 of the second byte. */
    wr8(m, flags + 1u, rd8(m, flags + 1u) & 0xF7u);
    wr8(m, flags + 1u, rd8(m, flags + 1u) | 0x04u);
    if ((int16_t)rd16(m, a5 + 0x30D2u) < 0xF0) {
        wr32(m, obj, 0x11EEFEu); /* 0x11EEF6 */
        return;
    }

    wr8(m, flags + 1u, rd8(m, flags + 1u) | 0x08u); /* 0x11EF2E */
    wr16(m, obj + 6u, 0xAu);
    wr8(m, flags, rd8(m, flags) | 0x08u);
    if (rd8(m, a5 + 0x3260u) == 0)
        wr8(m, flags, rd8(m, flags) | 0x10u);
    wr16(m, obj + 0x20u, 0x8000u); /* 0x11EF4E */
    wr16(m, obj + 0x42u, 0x80u);
    wr32(m, obj + 0x18u, 0u);
    wr32(m, obj, 0x11EF66u);
}

int Fn_11F266(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a5 = r->a[5];
    if (rd8(m, a5 + 0x1D58u) & 2u) {
        wr32(m, r->a[7] - 4u, 0x11F274u); /* 0x11F26E */
        uint32_t random = Rng_NextMem(m);
        uint16_t delta = (uint16_t)((random & 3u) - 1u);
        r->d[0] = (random & 0xFFFF0000u) | delta;
        uint16_t x = rd16(m, a5 + 0x30D2u);
        wr16(m, a5 + 0x30D2u, (uint16_t)((int16_t)x <= 0xA0
                                         ? x + delta : x - delta));
    }

    uint16_t timer = rd16(m, obj + 6u); /* 0x11F28C */
    wr16(m, obj + 6u, (uint16_t)(timer - 1u));
    if ((int16_t)timer > 1)
        wr32(m, obj, 0x11F266u); /* 0x11F25E */
    else
        finish_wait(m, obj, a5);
    r->a[7] += 4u;
    return 0;
}
