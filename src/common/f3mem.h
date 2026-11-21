#ifndef TAITOF3_COMMON_F3MEM_H
#define TAITOF3_COMMON_F3MEM_H
/* Big-endian views of the 68EC020 address space used by decompiled code that
 * is verified against MAME RAM captures: Work RAM image (0x400000, 128 KiB)
 * plus the program ROM (0x000000, 2 MiB) for constant tables. */
#include <stdint.h>

#define F3_ROM_BASE   0x000000u
#define F3_ROM_SIZE   0x200000u
#define F3_WRAM_BASE  0x400000u
#define F3_WRAM_SIZE  0x020000u
#define F3_A5         0x408000u  /* global base register */
#define F3_DPRAM_BASE 0xC00000u  /* sound DPRAM (MB8421, 8-bit) */
#define F3_DPRAM_SIZE 0x000800u
#define F3_SPR_BASE   0x600000u  /* sprite RAM */
#define F3_SPR_SIZE   0x010000u

typedef struct {
    uint8_t *wram;        /* F3_WRAM_SIZE bytes, big-endian */
    const uint8_t *rom;   /* F3_ROM_SIZE bytes, big-endian */
    uint8_t *dpram;       /* optional, F3_DPRAM_SIZE bytes */
    uint8_t *spr;         /* optional, F3_SPR_SIZE bytes (big-endian words) */
} f3mem_t;

uint8_t  *f3_ptr(const f3mem_t *m, uint32_t addr); /* abort()s outside RAM/ROM */
uint8_t   rd8(const f3mem_t *m, uint32_t a);
uint16_t  rd16(const f3mem_t *m, uint32_t a);
uint32_t  rd32(const f3mem_t *m, uint32_t a);
void      wr8(const f3mem_t *m, uint32_t a, uint8_t v);
void      wr16(const f3mem_t *m, uint32_t a, uint16_t v);
void      wr32(const f3mem_t *m, uint32_t a, uint32_t v);

#endif
