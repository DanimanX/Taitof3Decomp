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

typedef struct {
    uint8_t *wram;        /* F3_WRAM_SIZE bytes, big-endian */
    const uint8_t *rom;   /* F3_ROM_SIZE bytes, big-endian */
} f3mem_t;

uint8_t  *f3_ptr(const f3mem_t *m, uint32_t addr); /* abort()s outside RAM/ROM */
uint8_t   rd8(const f3mem_t *m, uint32_t a);
uint16_t  rd16(const f3mem_t *m, uint32_t a);
uint32_t  rd32(const f3mem_t *m, uint32_t a);
void      wr8(const f3mem_t *m, uint32_t a, uint8_t v);
void      wr16(const f3mem_t *m, uint32_t a, uint16_t v);
void      wr32(const f3mem_t *m, uint32_t a, uint32_t v);

#endif
