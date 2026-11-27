/* ROM 0x15894E: second half of the stage line-RAM setup. Eleven times it
 * reloads the line table base 0x62B000 + A5-0x5D56 and sets the priority
 * nibble (bits 3..0) of a row range with 0x17668A: rows 00-17 = 0xA,
 * 18-37 = 8, 35-47 = 0xA, 48-57 = 8, 58-5F = 0xA, 60-87 = 8, 88-8F = 0xA,
 * 90-9F = 8, A0-AF = 0xA, B0-CF = 8, D0-E7 = 0xA. Ranges 35-37 overlap on
 * purpose (the later block wins). Saves A0/D6/D7 only, so D0.w ends = 0xA (the
 * last block's nibble word) and D1.w = the last line word written. */
#include "fn_auto.h"

/* ROM 0x0017668A: for rows D7.w..D6.w set bits 3..0 of the word at
 * A0 + D7.w*2 to D0.w; returns the last word written (left in D1.w). */
static uint16_t f15894e_prio_17668A(const f3mem_t *m, uint32_t a0,
                                    uint16_t start, uint16_t end, uint16_t nib)
{
    uint16_t d7 = start;
    uint16_t d1 = 0u;

    for (;;) {                                                      /* 0x17668A */
        uint32_t p = a0 + (uint32_t)(int32_t)(int16_t)d7 * 2u;
        d1 = rd16(m, p);                                            /* 0x17668A */
        d1 = (uint16_t)((d1 & 0xFFF0u) | (nib & 0x000Fu));          /* 0x17668E bfins */
        wr16(m, p, d1);                                             /* 0x176692 */
        d7++;                                                       /* 0x176696 */
        if ((int16_t)d7 > (int16_t)end)                             /* 0x176698..0x17669A */
            break;
    }
    return d1;
}

int Fn_15894E(const f3mem_t *m, f3regs_t *r)
{
    static const struct { uint16_t first, last, nibble; } blocks[11] = {
        { 0x00u, 0x17u, 0xAu }, { 0x18u, 0x37u, 0x8u }, { 0x35u, 0x47u, 0xAu },
        { 0x48u, 0x57u, 0x8u }, { 0x58u, 0x5Fu, 0xAu }, { 0x60u, 0x87u, 0x8u },
        { 0x88u, 0x8Fu, 0xAu }, { 0x90u, 0x9Fu, 0x8u }, { 0xA0u, 0xAFu, 0xAu },
        { 0xB0u, 0xCFu, 0x8u }, { 0xD0u, 0xE7u, 0xAu },
    };
    uint16_t last = 0u;

    for (unsigned i = 0; i < 11u; i++) {
        /* each block reloads A0 and pushes {start, end} (popped end first) */
        uint32_t a0 = 0x0062B000u
                    + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
        last = f15894e_prio_17668A(m, a0, blocks[i].first, blocks[i].last,
                                   blocks[i].nibble);
    }
    /* 0x158B06 rts: A0/D6/D7 were restored by movem; D0.w = the last nibble
     * word, D1.w = the last line word. */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | 0x000Au;
    r->d[1] = (r->d[1] & 0xFFFF0000u) | last;
    return 0;
}
