#include "fn_auto.h"
/* ROM 0x11F0F4: UNVERIFIED (no capture): jitter the shared X position when enabled and advance the timed state. */

int Fn_11F0F4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a5 = r->a[5];

    if (rd8(m, a5 + 0x1D58u) & 2u) {
        r->a[7] -= 4u;
        wr32(m, r->a[7], 0x11F102u);
        uint32_t random = Rng_NextMem(m); /* 0x11F0FC */
        r->a[7] += 4u; /* 0x001D9E RTS */
        uint16_t step = (uint16_t)((random & 3u) - 1u);
        r->d[0] = (random & 0xFFFF0000u) | step;
        uint16_t x = rd16(m, a5 + 0x30D2u);
        if ((int16_t)x > 0xA0)
            wr16(m, a5 + 0x30D2u, (uint16_t)(x - step)); /* 0x11F110 */
        else
            wr16(m, a5 + 0x30D2u, (uint16_t)(x + step)); /* 0x11F116 */
    }

    uint16_t timer = rd16(m, obj + 6u);
    wr16(m, obj + 6u, (uint16_t)(timer - 1u));
    if ((int16_t)timer > 1) {
        wr32(m, obj, 0x11F0F4u); /* 0x11F0EC */
    } else {
        wr16(m, a5 + 0x30D6u, (uint16_t)(rd16(m, a5 + 0x30D6u) - 1u));
        wr16(m, obj + 6u, 0x1Eu);
        wr32(m, obj, 0x11F132u); /* 0x11F12A */
    }
    r->a[7] += 4u;
    return 0;
}
