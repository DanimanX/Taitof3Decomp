#include "fn_auto.h"

/* ROM 0x118242: verified on a capture from the Y04 re-survey states: initialize the sprite and jitter its packed coordinates. */

/* ROM 0x11825E / 0x11826E: call the RNG and add its signed jitter word. */
static void jitter_coordinate(const f3mem_t *m, f3regs_t *r,
                              uint32_t coord, uint32_t return_pc)
{
    wr32(m, r->a[7] - 4u, return_pc);
    uint32_t random = Rng_NextMem(m);
    uint16_t delta = (uint16_t)((random & 7u) - 4u);
    r->d[0] = (random & 0xFFFF0000u) | delta;
    wr16(m, coord, (uint16_t)(rd16(m, coord) + delta));
}

int Fn_118242(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t x = obj + 0x52u;
    uint32_t y = obj + 0x54u;
    wr32(m, x, 0u); /* 0x118242 and 0x118246 clear overlapping longs. */
    wr32(m, y, 0u);
    r->d[0] = rd32(m, obj + 0x0Cu);
    r->d[1] = (uint32_t)(int32_t)(int16_t)(r->d[0] >> 16);
    wr16(m, x, (uint16_t)r->d[1]);
    r->d[1] = (uint32_t)(int32_t)(int16_t)r->d[0];
    wr16(m, y, (uint16_t)r->d[1]);

    jitter_coordinate(m, r, x, 0x118264u);
    jitter_coordinate(m, r, y, 0x118274u);
    wr16(m, obj + 0x1Cu, 0u); /* 0x11827E */
    wr16(m, obj + 0x1Eu, 0u);
    wr32(m, obj + 0x10u, 0x04F1B4u);
    wr16(m, x, (uint16_t)(rd16(m, x) - 0x10u));
    wr16(m, y, (uint16_t)(rd16(m, y) - 0x10u));
    wr16(m, obj + 0x22u, 0x0100u);
    wr32(m, obj, 0x1182A8u); /* 0x1182A0 fall-through */
    r->a[7] += 4u;
    return 0;
}
