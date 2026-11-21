#include "f3mem.h"
#include <stdio.h>
#include <stdlib.h>

uint8_t *f3_ptr(const f3mem_t *m, uint32_t a)
{
    a &= 0xFFFFFFu;                      /* 24-bit bus */
    if (a >= 0x420000u && a < 0x440000u) /* WORK_RAM mirror */
        a -= 0x20000u;
    if (a >= F3_WRAM_BASE && a < F3_WRAM_BASE + F3_WRAM_SIZE)
        return &m->wram[a - F3_WRAM_BASE];
    if (m->dpram && a >= F3_DPRAM_BASE && a < F3_DPRAM_BASE + F3_DPRAM_SIZE)
        return &m->dpram[a - F3_DPRAM_BASE];
    if (a < F3_ROM_SIZE && m->rom)
        return (uint8_t *)&m->rom[a];
    fprintf(stderr, "f3mem: access outside RAM/ROM at %06X\n", a);
    abort();
}

uint8_t rd8(const f3mem_t *m, uint32_t a) { return *f3_ptr(m, a); }
uint16_t rd16(const f3mem_t *m, uint32_t a)
{
    const uint8_t *p = f3_ptr(m, a);
    return (uint16_t)(p[0] << 8 | p[1]);
}
uint32_t rd32(const f3mem_t *m, uint32_t a)
{
    return (uint32_t)rd16(m, a) << 16 | rd16(m, a + 2);
}
void wr8(const f3mem_t *m, uint32_t a, uint8_t v) { *f3_ptr(m, a) = v; }
void wr16(const f3mem_t *m, uint32_t a, uint16_t v)
{
    uint8_t *p = f3_ptr(m, a);
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)v;
}
void wr32(const f3mem_t *m, uint32_t a, uint32_t v)
{
    wr16(m, a, (uint16_t)(v >> 16));
    wr16(m, a + 2, (uint16_t)v);
}
