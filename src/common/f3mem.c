#include <string.h>
#include "f3mem.h"
#include <stdio.h>
#include <stdlib.h>

void f3_eeprom_advance(f3_eeprom_t *e, uint32_t ns)
{
    if (e) e->busy_ns = e->busy_ns > ns ? e->busy_ns - ns : 0;
}

uint8_t f3_eeprom_do(const f3_eeprom_t *e)
{
    if (e->phase == F3_EEP_START) return e->busy_ns == 0;
    if (e->phase == F3_EEP_READ) return (uint8_t)(e->shift >> 31);
    return 1; /* pull-up while DO is high impedance */
}

static void eeprom_program(f3_eeprom_t *e)
{
    if (!e->write_enabled) {
        e->phase = F3_EEP_IDLE;
        return;
    }
    /* MAME changes the cells immediately and times the ready/busy line. */
    switch (e->command) {
    case F3_EEP_WRITE_WORD:
        e->words[e->address] = (uint16_t)e->shift;
        e->busy_ns = 1750000;
        break;
    case F3_EEP_ERASE_WORD:
        e->words[e->address] = UINT16_MAX;
        e->busy_ns = 1000000;
        break;
    case F3_EEP_WRITE_ALL:
        for (unsigned i = 0; i < F3_EEP_WORDS; ++i)
            e->words[i] &= (uint16_t)e->shift;
        e->busy_ns = 8000000;
        break;
    case F3_EEP_ERASE_ALL:
        for (unsigned i = 0; i < F3_EEP_WORDS; ++i)
            e->words[i] = UINT16_MAX;
        e->busy_ns = 8000000;
        break;
    }
    e->phase = F3_EEP_COMPLETE;
}

static void eeprom_decode(f3_eeprom_t *e)
{
    static const uint8_t commands[] = { F3_EEP_INVALID, F3_EEP_WRITE_WORD,
        F3_EEP_READ_WORD, F3_EEP_ERASE_WORD };
    static const uint8_t special[] = { F3_EEP_EWDS, F3_EEP_WRITE_ALL,
        F3_EEP_ERASE_ALL, F3_EEP_EWEN };
    e->address = e->command_bits & (F3_EEP_WORDS - 1);
    e->command = commands[e->command_bits >> 6];
    if (e->command == F3_EEP_INVALID) {
        e->command = special[e->address >> 4];
        e->address = 0;
    }
    e->bits = 0;
    switch (e->command) {
    case F3_EEP_READ_WORD:
        e->shift = 0; /* dummy zero before the first data clock */
        e->phase = F3_EEP_READ;
        break;
    case F3_EEP_WRITE_WORD:
    case F3_EEP_WRITE_ALL:
        e->shift = 0;
        e->phase = F3_EEP_DATA;
        break;
    case F3_EEP_EWDS:
    case F3_EEP_EWEN:
        e->write_enabled = e->command == F3_EEP_EWEN;
        e->phase = F3_EEP_IDLE;
        break;
    default:
        eeprom_program(e);
        break;
    }
}

static void eeprom_port_write(f3_eeprom_t *e, uint8_t pins)
{
    uint8_t changed = pins ^ e->pins;
    e->pins = pins & (F3_EEP_CS | F3_EEP_CLK | F3_EEP_DI);
    if (changed & F3_EEP_CS) {
        e->phase = (pins & F3_EEP_CS) ? F3_EEP_START : F3_EEP_IDLE;
        return;
    }
    if (!(changed & pins & F3_EEP_CLK)) return;

    unsigned di = (pins & F3_EEP_DI) != 0;
    switch (e->phase) {
    case F3_EEP_START:
        if (di && !e->busy_ns) {
            e->bits = e->command_bits = 0;
            e->phase = F3_EEP_COMMAND;
        }
        break;
    case F3_EEP_COMMAND:
        e->command_bits = (e->command_bits << 1) | di;
        if (++e->bits == 8) eeprom_decode(e);
        break;
    case F3_EEP_READ:
        /* F3 uses the non-streaming part, MSB first on rising CLK. */
        if (e->bits++ == 0)
            e->shift = (uint32_t)e->words[e->address] << 16;
        else
            e->shift = (e->shift << 1) | 1u;
        break;
    case F3_EEP_DATA:
        e->shift = (e->shift << 1) | di;
        if (++e->bits == 16) eeprom_program(e);
        break;
    }
}

uint8_t *f3_ptr(const f3mem_t *m, uint32_t a)
{
    a &= 0xFFFFFFu;                      /* 24-bit bus */
    if (a >= 0x420000u && a < 0x440000u) /* WORK_RAM mirror */
        a -= 0x20000u;
    if (a >= F3_WRAM_BASE && a < F3_WRAM_BASE + F3_WRAM_SIZE)
        return &m->wram[a - F3_WRAM_BASE];
    if (m->pal && a >= F3_PAL_BASE && a < F3_PAL_BASE + F3_PAL_SIZE)
        return &m->pal[a - F3_PAL_BASE];
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

uint8_t rd8(const f3mem_t *m, uint32_t a)
{
    if (m->eeprom && (a & 0xFFFFFEu) == (F3_EEP_IN - 1u))
        return f3_eeprom_do(m->eeprom);
    return *f3_ptr(m, a);
}
uint16_t rd16(const f3mem_t *m, uint32_t a)
{
    if (m->eeprom && ((a & 0xFFFFFFu) == F3_EEP_IN - 1u || (a & 0xFFFFFFu) == F3_EEP_IN))
        return (uint16_t)(rd8(m, a) << 8 | rd8(m, a + 1));
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
    if (m->eeprom && (a & 0xFFFFFFu) == F3_EEP_OUT) {
        eeprom_port_write(m->eeprom, v);
        return;
    }
    *f3_ptr(m, a) = v;
}
void wr16(const f3mem_t *m, uint32_t a, uint16_t v)
{
    if (is_rom(a)) return;
    if (m->eeprom && (a & 0xFFFFFEu) == (F3_EEP_OUT - 1u)) {
        eeprom_port_write(m->eeprom, (uint8_t)((a & 1u) ? v >> 8 : v));
        return;
    }
    uint8_t *p = f3_ptr(m, a);
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)v;
}
void wr32(const f3mem_t *m, uint32_t a, uint32_t v)
{
    wr16(m, a, (uint16_t)(v >> 16));
    wr16(m, a + 2, (uint16_t)v);
}
