#include "fn_auto.h"
/* ROM 0x1849AA: verified: the capture discriminates (an empty body fails it): apply the ROM percentage table to the signed input word in D0. */

int Fn_1849AA(const f3mem_t *m, f3regs_t *r)
{
    /* 0x1849AA predecrements in descending register order. */
    wr32(m, r->a[7] - 4u, r->d[2]);
    wr32(m, r->a[7] - 8u, r->d[1]);
    uint32_t index = rd8(m, r->a[5] + 0x087Fu) >> 4; /* 0x1849AE */
    /* 0x1849B8 EA extension + 0x1E, D1.w scale 1. */
    uint32_t percent = rd8(m, 0x1849D6u + index);
    int32_t scaled = (int32_t)(int16_t)r->d[0] * 65536;
    /* 0x1849C0 extension 0x2802 selects a signed 32-bit dividend
     * and puts the quotient in D2. 0x1849C8 keeps the low 32 product bits. */
    uint32_t product = (uint32_t)(scaled / 100) * percent;
    uint16_t result = (uint16_t)((uint16_t)r->d[0] + (product >> 16));
    r->d[0] = (r->d[0] & 0xFFFF0000u) | result;
    r->a[7] += 4u;
    return 0;
}
