/* ROM 0x120CC8: boss debris/spark spawner init (A0 = object).
 * Places object near (A5+0x30D2 - 0x1E, A5+0x30D6 - 0x1C) with RNG jitter
 * (divided by 4 when A5+0x1D58 bit 1 set), sets sprite 0x00062D98, palette/flip
 * 0x0000/0xE8E8, +0x42 = 0x000E, and arms 0x120D30. */
#include "fn_auto.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_120CC8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];

    wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x30D2u));     /* 0x120CC8 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x001Eu)); /* 0x120CCE */
    wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x30D6u));     /* 0x120CD4 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x001Cu)); /* 0x120CDA */

    d0 = Rng_NextMem(m);                                /* 0x120CE0 */
    d0 = setw(d0, (uint16_t)d0 & 0x00FFu);              /* 0x120CE6 */
    d1 = setw(d1, (uint16_t)d0);                        /* 0x120CEA */

    d0 = Rng_NextMem(m);                                /* 0x120CEC */
    d0 = setw(d0, (uint16_t)d0 & 0x00FFu);              /* 0x120CF2 */
    d0 = setw(d0, (uint16_t)((uint16_t)d0 - 0x0080u));  /* 0x120CF6 */

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {      /* 0x120CFA */
        d1 = setw(d1, (uint16_t)((int16_t)(uint16_t)d1 >> 2)); /* 0x120D04 */
        d0 = setw(d0, (uint16_t)((int16_t)(uint16_t)d0 >> 2)); /* 0x120D06 */
    }

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - (uint16_t)d1)); /* 0x120D08 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d0)); /* 0x120D0C */

    wr32(m, obj + 0x10u, 0x00062D98u);                  /* 0x120D10 */
    wr16(m, obj + 0x1Cu, 0x0000u);                      /* 0x120D18 */
    wr16(m, obj + 0x1Eu, 0xE8E8u);                      /* 0x120D1C */
    wr16(m, obj + 0x42u, 0x000Eu);                      /* 0x120D22 */
    wr32(m, obj, 0x00120D30u);                          /* 0x120D28 */

    r->d[0] = d0;
    r->d[1] = d1;
    return 0;
}
