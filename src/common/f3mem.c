#include <string.h>
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
    if (m->spr && a >= F3_SPR_BASE && a < F3_SPR_BASE + F3_SPR_SIZE)
        return &m->spr[a - F3_SPR_BASE];
    if (m->gfx && a >= F3_GFX_BASE && a < F3_GFX_BASE + F3_GFX_SIZE)
        return &m->gfx[a - F3_GFX_BASE];
    if (m->dpram && a >= F3_DPRAM_BASE && a < F3_DPRAM_BASE + F3_DPRAM_SIZE)
        return &m->dpram[a - F3_DPRAM_BASE];
    if (a < F3_ROM_SIZE && m->rom)
        return &((uint8_t *)m->rom)[a];
    /* Unmapped: I/O space (0x4A0000..), cart/user windows, or a corrupt
     * pointer read out of a capture. Returning a zeroed scratch byte keeps a
     * single bad address from abort()ing the whole harness (one such capture
     * took 118B74 down and hid every later verdict). The write is discarded,
     * so the owning routine reports a mismatch instead of crashing the run. */
    static uint8_t unmapped[8];
    /* cleared on every access: a write stored here must not leak into a
     * later read (another routine, another test entry) of unmapped space */
    memset(unmapped, 0, sizeof unmapped);
    fprintf(stderr, "f3mem: access outside RAM/ROM at %06X\n", a);
    return unmapped;
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
/* Writes to ROM are ignored by the hardware. f3_ptr hands back a pointer into
 * the ROM image, which every test entry shares: storing through it would let a
 * routine that strays into ROM corrupt the code/data later entries read. */
static int is_rom(uint32_t a) { return (a & 0xFFFFFFu) < F3_ROM_SIZE; }
void wr8(const f3mem_t *m, uint32_t a, uint8_t v)
{
    if (is_rom(a)) return;
    *f3_ptr(m, a) = v;
}
void wr16(const f3mem_t *m, uint32_t a, uint16_t v)
{
    if (is_rom(a)) return;
    uint8_t *p = f3_ptr(m, a);
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)v;
}
void wr32(const f3mem_t *m, uint32_t a, uint32_t v)
{
    wr16(m, a, (uint16_t)(v >> 16));
    wr16(m, a + 2, (uint16_t)v);
}
