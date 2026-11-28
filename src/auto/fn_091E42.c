#include "fn_auto.h"
/* ROM 0x091E42: draw the three ON/OFF and NORMAL/CENTER test-menu labels of
 * one option. D0 = 7 - (A5-0xC64) selects one bit of the option byte at
 * A5-0xC4D; 0x091EA4 (reading A5-0x5DDB bits 3 and 2) may force that bit and
 * 0x091E86 writes it back. The three labels go to text RAM 0x61C136,
 * 0x61C1B6 and 0x61C234 with the bits 7, 6 and 5 of the byte: 0x091EC2 draws
 * "ON " (colour 8) or "OFF" (colour 3), 0x091F0C draws "NORM" or "CENT"
 * (colour 6). D0 and A0 are saved; A1 ends at 0x61C136. */
#include "../kernel/fn_010352.h"   /* Kernel_005666 (0x005666) */

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* bit `off` of the bit stream that starts at the MSB of addr. */
static uint32_t bf_get(const f3mem_t *m, uint32_t addr, uint32_t off)
{
    uint8_t b = rd8(m, addr + (off >> 3));
    return (uint32_t)((b >> (7u - (off & 7u))) & 1u);
}

static void bf_put(const f3mem_t *m, uint32_t addr, uint32_t off, uint32_t v)
{
    uint32_t a = addr + (off >> 3);
    uint8_t mask = (uint8_t)(1u << (7u - (off & 7u)));
    uint8_t b = rd8(m, a);
    wr8(m, a, v ? (uint8_t)(b | mask) : (uint8_t)(b & ~mask)); /* 0x091E56 bfins */
}

int Fn_091E42(const f3mem_t *m, f3regs_t *r)
{
    uint32_t d0 = 7u & 0xFFFFFFFFu;                        /* 0x091E46 moveq #7,D0 */
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 - rd16(m, F3_A5 - 0x0C64u)); /* 0x091E48 */
    uint32_t off = (uint16_t)d0;

    uint32_t d1 = bf_get(m, F3_A5 - 0x0C4Du, off);         /* 0x091E4C bfextu {D0:1} */

    /* 0x091E52 bsr 0x091EA4 */
    uint8_t flags = rd8(m, F3_A5 - 0x5DDBu);
    if ((flags & 0x08u) == 0u)                             /* 0x091EA4 btst #3 ; bne */
        d1 = 1u;                                           /* 0x091EAE */
    if ((flags & 0x04u) == 0u)                             /* 0x091EB2 btst #2 ; bne */
        d1 = 0u;                                           /* 0x091EBC */

    bf_put(m, F3_A5 - 0x0C4Du, off, d1 & 1u);              /* 0x091E56 bfins */

    uint16_t d2w = 0x001Bu;                                /* 0x091E62 */
    uint16_t d3w = 0x0002u;                                /* 0x091E66 */
    d2w = (uint16_t)(d2w + d2w);                           /* 0x091E6A add.w D2w,D2w */
    d3w = (uint16_t)(d3w << 7);                            /* 0x091E6C lsl.w #7,D3w */
    d2w = (uint16_t)(d2w + d3w);                           /* 0x091E6E add.w D3w,D2w */
    uint32_t a1 = 0x0061C000u + (uint32_t)(int32_t)(int16_t)d2w; /* 0x091E5C..0x091E70 */
    uint32_t d0v = a1;                                     /* 0x091E72 move.l A1,D0 */

    /* 0x091E74..0x091E7A: 0x091EC2 with bit 7 */
    d1 = bf_get(m, F3_A5 - 0x0C4Du, 7u);
    if ((uint16_t)d1 != 0u)                                /* 0x091EC6 tst.w D1w ; bne */
        Kernel_005666(m, 0x00091F02u, d0v, 8u);            /* 0x091EE6..0x091EF8: "ON " */
    else
        Kernel_005666(m, 0x00091F06u, d0v, 3u);            /* 0x091ECC..0x091EE2: "OFF" */

    d0v = (d0v + 0x80u) & 0xFFFFFFFFu;                     /* 0x091E7E addi.l #0x80,D0 */

    /* 0x091E84..0x091E8A: 0x091EC2 with bit 6 */
    d1 = bf_get(m, F3_A5 - 0x0C4Du, 6u);
    if ((uint16_t)d1 != 0u)
        Kernel_005666(m, 0x00091F02u, d0v, 8u);
    else
        Kernel_005666(m, 0x00091F06u, d0v, 3u);

    d0v = (d0v + 0x7Eu) & 0xFFFFFFFFu;                     /* 0x091E8E addi.l #0x7E,D0 */

    /* 0x091E94..0x091E9A: 0x091F0C with bit 5 */
    d1 = bf_get(m, F3_A5 - 0x0C4Du, 5u);
    if ((uint16_t)d1 != 0u)                                /* 0x091F10 tst.w D1w ; bne */
        Kernel_005666(m, 0x00091F51u, d0v, 6u);            /* 0x091F30..0x091F42: "CENT" */
    else
        Kernel_005666(m, 0x00091F4Cu, d0v, 6u);            /* 0x091F16..0x091F2C: "NORM" */

    r->d[1] = d1;                                          /* bfextu clears all of D1 */
    r->d[2] = setw(r->d[2], d2w);
    r->d[3] = setw(r->d[3], d3w);
    r->a[1] = a1;                                          /* 0x091E70 adda.w D2w,A1 */
    return 0;                                              /* 0x091EA2 rts */
}
