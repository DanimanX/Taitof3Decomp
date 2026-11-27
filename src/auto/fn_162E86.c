/* ROM 0x162E86: paint the three line-RAM bands with a solid fill via
 * 0x176198 -- 0x62B000 <- 0x700D, 0x62B400 <- 0x300A, 0x626200 <- 0xBBBB
 * (move.w #-0x4445) -- then arm the tick 0x162ED2. Every register survives:
 * the fills are wrapped in movem.l {A0}/movem.l (SP)+,{A0} and 0x176198 saves
 * D0-D7. */
#include "fn_auto.h"

/* ROM 0x176198: fill the 0xE8 words (0x1D0 bytes) at base + (A5-0x5D56) with
 * val. The ROM writes them backwards from base+off+0x1D0 with movem.l bursts
 * (0x1761C4..0x1761FC); every byte of the range gets the same value, so the
 * order does not matter. */
static void fill_176198(const f3mem_t *m, uint32_t base, uint16_t val)
{
    uint32_t a0 = base + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    for (uint32_t off = 0u; off < 0x01D0u; off += 2u)
        wr16(m, a0 + off, val);
}

int Fn_162E86(const f3mem_t *m, f3regs_t *r)
{
    fill_176198(m, 0x0062B000u, 0x700Du);         /* ROM 0x162E86..0x162E9A */
    fill_176198(m, 0x0062B400u, 0x300Au);         /* ROM 0x162E9E..0x162EAE */
    fill_176198(m, 0x00626200u, 0xBBBBu);         /* ROM 0x162EB2..0x162EC2 */

    wr32(m, r->a[0], 0x00162ED2u);                /* ROM 0x162ECA move.l #0x162ED2,(A0) */
    return 0;                                     /* ROM 0x162ED0 rts */
}
