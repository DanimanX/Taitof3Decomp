#include "fn_auto.h"

/* ROM 0x176710: verified: the capture discriminates (an empty body fails it): write the first line word and update the low nibble on subsequent lines. */
int Fn_176710(const f3mem_t *m, f3regs_t *r)
{
    uint16_t line = (uint16_t)r->d[7];
    const int16_t last = (int16_t)r->d[6];
    const uint16_t value = (uint16_t)r->d[0];

    /* 0x176710: extension 0x7200 indexes by signed D7.w times two. */
    wr16(m, r->a[0] + (uint32_t)((int32_t)(int16_t)line * 2), value);
    ++line;
    /* 0x176718 branches to 0x17668A, where BFINS replaces bits 3:0. */
    while ((int16_t)line <= last) {
        uint32_t addr = r->a[0] + (uint32_t)((int32_t)(int16_t)line * 2);
        uint16_t word = (rd16(m, addr) & 0xFFF0u) | (value & 0xFu);
        r->d[1] = (r->d[1] & 0xFFFF0000u) | word;
        wr16(m, addr, word);
        ++line;
    }
    r->d[7] = (r->d[7] & 0xFFFF0000u) | line;
    r->a[7] += 4u;
    return 0;
}
