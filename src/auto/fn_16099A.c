#include "fn_auto.h"

/* ROM 0x16099A: UNVERIFIED (weak capture): calculate the perspective boundary from the scroll word. */
int Fn_16099A(const f3mem_t *m, f3regs_t *r)
{
    uint16_t y = (0x7Fu - rd16(m, r->a[5] + 0x4218u)) & 0xFFu;
    uint32_t distance = y <= 0x73u ? 0x74u - y : y - 0x73u;
    /* DIVU's quotient fits in a word. MULU reads only that word and uses
     * the unsigned immediate 0xE000 from 0x1609C0 / 0x1609DC. */
    uint32_t scaled = ((distance << 6) / 0x28u) * 0xE000u << 1;
    r->d[1] = (scaled << 16) | (scaled >> 16); /* SWAP at 0x1609C6 / 0x1609E2 */
    uint16_t correction = (uint16_t)r->d[1];
    y = (uint16_t)(y <= 0x73u ? y - correction : y + correction);
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (uint16_t)(y - 0xCu);
    r->a[7] += 4u;
    return 0;
}
