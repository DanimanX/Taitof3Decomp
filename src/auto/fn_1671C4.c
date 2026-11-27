/* ROM 0x1671C4: stage-intro setup (A0 = object). Fills 0x1D0 bytes of line
 * RAM at 0x627600 + (A5-0x5D56) with 0xDDDD, clears the layer scroll/offset
 * longs at A5-0x5DA4 / A5+0x4208 / A5+0x4224 / A5+0x4228, masks the two
 * layer control longs (A5-0x5DA8, A5+0x4204) down to their top 7 bits, copies
 * 0x20 longs from 0x167FEE to 0x40B42E and 0x10 longs from 0x16806E to
 * 0x40B4AE, arms two 0x80-frame palette ramps (gfx data 0x23AC0 -> palette
 * 0xF80 with 0x20 colours, 0x23F40 -> 0x1400 with 0x10), and re-arms the
 * object with 0x1672A8.
 *
 * Registers at rts: D0 = the second 0x183C result (claimed ramp slot, or
 * 0xFFFFFFFF when no slot was free), A0 = the entry A0; the two movem
 * save/restore pairs around the copies put A1, D1..D7 and A2..A6 back to
 * their entry values, which is why the capture's post A1 is the object's own
 * entry A1 rather than the copy destination. */
#include "fn_auto.h"
#include "../objects/obj_common.h"

/* ROM 0x176198: fill the 0x1D0 bytes at base + (A5-0x5D56) with val. The ROM
 * builds a long from the word argument (move.w -/(SP) ; swap ; move.w (SP)+)
 * and stores it with 14 movem.l bursts of 8 longs plus one of 4, walking down
 * from base+off+0x1D0; the whole range gets the same value. */
static void fn1671c4_fill176198(const f3mem_t *m, uint32_t base, uint16_t val)
{
    uint32_t a0 = base + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    for (uint32_t off = 0u; off < 0x01D0u; off += 2u)
        wr16(m, a0 + off, val);                     /* ROM 0x1761C4..0x1761FC */
}

/* ROM 0x16A19C: copy d0+1 longs from a0 to a1 (d0 is the caller's count-1) */
static void fn1671c4_copy(const f3mem_t *m, uint32_t src, uint32_t dst, uint16_t d0)
{
    for (uint32_t i = 0u; i <= (uint32_t)d0; i++)
        wr32(m, dst + i * 4u, rd32(m, src + i * 4u));
}

/* 0x00183C wraps 0x001792: returns the claimed ramp slot, 0xFFFFFFFF if none */
static uint32_t fn1671c4_ramp(const f3mem_t *m, uint32_t src, uint32_t ramp_src,
                              uint32_t dst, uint32_t frames, uint32_t count)
{
    uint32_t slot = Pal_QueueRamp(m, src, ramp_src, dst, frames, count);
    return slot ? slot : 0xFFFFFFFFu;
}

int Fn_1671C4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];                                  /* 0x1671C4 movem */

    /* 0x1671C8..0x1671DC: jsr 0x176198(0x627600, 0xDDDD) */
    fn1671c4_fill176198(m, 0x00627600u, 0xDDDDu);

    wr32(m, F3_A5 - 0x5DA4u, 0u);                            /* 0x1671E0 */
    wr32(m, F3_A5 + 0x4208u, 0u);                            /* 0x1671E4 */
    wr32(m, F3_A5 - 0x5DA8u, rd32(m, F3_A5 - 0x5DA8u) & 0xFE000000u); /* 0x1671E8 */
    wr32(m, F3_A5 + 0x4204u, rd32(m, F3_A5 + 0x4204u) & 0xFE000000u); /* 0x1671F0 */
    wr32(m, F3_A5 + 0x4224u, 0u);                            /* 0x1671F8 */
    wr32(m, F3_A5 + 0x4228u, 0u);                            /* 0x1671FC */

    /* 0x167200..0x167224 / 0x167228..0x167248: two ROM->RAM copies */
    fn1671c4_copy(m, 0x00167FEEu, 0x0040B42Eu, 0x1Fu);   /* D0 = 0x20-1 */
    fn1671c4_copy(m, 0x0016806Eu, 0x0040B4AEu, 0x0Fu);   /* D0 = 0x10-1 */

    /* 0x16724C: jsr 0x183C(0x23AC0, 0x167FEE, 0xF80, 0x80, 0x20) */
    (void)fn1671c4_ramp(m, 0x00023AC0u, 0x00167FEEu, 0x0F80u, 0x80u, 0x20u);

    /* 0x167274: jsr 0x183C(0x23F40, 0x16806E, 0x1400, 0x80, 0x10) */
    r->d[0] = fn1671c4_ramp(m, 0x00023F40u, 0x0016806Eu, 0x1400u, 0x80u, 0x10u);

    wr32(m, obj, 0x001672A8u);                               /* 0x16729C..0x1672A0 */
    r->a[0] = obj;
    return 0;                                                /* 0x1672A6 rts */
}
