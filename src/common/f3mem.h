#ifndef TAITOF3_COMMON_F3MEM_H
#define TAITOF3_COMMON_F3MEM_H
/* Big-endian views of the 68EC020 address space used by decompiled code that
 * is verified against MAME RAM captures: Work RAM image (0x400000, 128 KiB)
 * plus the program ROM (0x000000, 2 MiB) and optional capture regions. */
#include <stdint.h>

#define F3_ROM_BASE   0x000000u
#define F3_ROM_SIZE   0x200000u
#define F3_WRAM_BASE  0x400000u
#define F3_WRAM_SIZE  0x020000u
#define F3_A5         0x408000u  /* global base register */
#define F3_PAL_BASE   0x440000u  /* palette RAM (0x440000-0x447FFF) */
#define F3_PAL_SIZE   0x008000u
#define F3_DPRAM_BASE 0xC00000u  /* sound DPRAM (MB8421, 8-bit) */
#define F3_DPRAM_SIZE 0x000800u
#define F3_SPR_BASE   0x600000u  /* sprite RAM */
#define F3_SPR_SIZE   0x010000u
#define F3_GFX_BASE   0x610000u  /* playfield, text, character and line RAM */
#define F3_GFX_SIZE   0x020000u

#define F3_EEP_IN     0x4A0001u
#define F3_EEP_OUT    0x4A0013u
#define F3_EEP_DI     0x04u
#define F3_EEP_CLK    0x08u
#define F3_EEP_CS     0x10u
#define F3_EEP_WORDS  64u

/* Phase and command values match the MAME 0.289 save items. */
enum f3_eeprom_phase {
    F3_EEP_IDLE, F3_EEP_START, F3_EEP_COMMAND, F3_EEP_READ,
    F3_EEP_DATA, F3_EEP_COMPLETE
};
enum f3_eeprom_command {
    F3_EEP_INVALID, F3_EEP_READ_WORD, F3_EEP_WRITE_WORD, F3_EEP_ERASE_WORD,
    F3_EEP_EWDS, F3_EEP_EWEN, F3_EEP_WRITE_ALL, F3_EEP_ERASE_ALL
};

typedef struct {
    uint16_t words[F3_EEP_WORDS];
    uint8_t phase, pins, write_enabled, command, address;
    uint32_t bits, command_bits, shift, busy_ns;
} f3_eeprom_t;

typedef struct {
    uint8_t *wram;        /* F3_WRAM_SIZE bytes, big-endian */
    const uint8_t *rom;   /* F3_ROM_SIZE bytes, big-endian */
    uint8_t *dpram;       /* optional, F3_DPRAM_SIZE bytes */
    uint8_t *spr;         /* optional, F3_SPR_SIZE bytes (big-endian words) */
    uint8_t *gfx;         /* optional, F3_GFX_SIZE bytes (0x610000-0x62FFFF) */
    uint8_t *pal;         /* optional, F3_PAL_SIZE bytes, big-endian */
    f3_eeprom_t *eeprom;  /* optional serial 93C46 */
} f3mem_t;

/* Translations advance device time where it matters, using emulated time. */
void f3_eeprom_advance(f3_eeprom_t *e, uint32_t ns);
uint8_t f3_eeprom_do(const f3_eeprom_t *e);

uint8_t  *f3_ptr(const f3mem_t *m, uint32_t addr); /* scratch buffer for unmapped addresses */
uint8_t   rd8(const f3mem_t *m, uint32_t a);
uint16_t  rd16(const f3mem_t *m, uint32_t a);
uint32_t  rd32(const f3mem_t *m, uint32_t a);
void      wr8(const f3mem_t *m, uint32_t a, uint8_t v);
void      wr16(const f3mem_t *m, uint32_t a, uint16_t v);
void      wr32(const f3mem_t *m, uint32_t a, uint32_t v);


/* ROM 0x001D8A Rng_Next on the RAM seed (0x406BAA): seed = seed*7+5,
 * returns the swapped seed (D0.w = high half). See src/kernel/rng.c. */
static inline uint32_t Rng_NextMem(const f3mem_t *m)
{
    uint32_t s = rd32(m, 0x406BAAu) * 7u + 5u;
    wr32(m, 0x406BAAu, s);
    return (s << 16) | (s >> 16);
}

#endif
