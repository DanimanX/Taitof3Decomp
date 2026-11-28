#include "fn_auto.h"
/* ROM 0x144720: close a gap in the 8-byte record list at A5+0x3320. It walks
 * the list until a record whose +0x06 word is negative, steps past it, and
 * while that position is below 0x40CAAC copies every earlier record one slot
 * up (word +0, word +2, word +4, byte +6, byte +7). The bottom record then
 * gets +0x02 = 0xA0, +0x04 = 0x80 and zeroed +0x06/+0x07. A0 ends at the
 * bottom record.
 * Writes into the ROM window are dropped: the ROM is read-only on the F3, so
 * a list pointer that lands there (as in the capture, where A5+0x3320 is 0)
 * has no effect on memory. */
static void put16(const f3mem_t *m, uint32_t a, uint16_t v)
{
    if (a >= F3_ROM_SIZE)
        wr16(m, a, v);
}

static void put8(const f3mem_t *m, uint32_t a, uint8_t v)
{
    if (a >= F3_ROM_SIZE)
        wr8(m, a, v);
}

int Fn_144720(const f3mem_t *m, f3regs_t *r)
{
    uint32_t top = rd32(m, F3_A5 + 0x3320u);       /* 0x144720 movea.l (0x3320,A5),A0 */
    uint32_t a0 = top;

    while ((int16_t)rd16(m, a0 + 0x06u) >= 0)      /* 0x144724 tst.w ; 0x144728 bmi */
        a0 += 8u;                                  /* 0x14472C lea (8,A0),A0 */
    a0 += 8u;                                      /* 0x144732 lea (8,A0),A0 */

    if ((int32_t)a0 < 0x0040CAAC) {                /* 0x144736 cmpa.l #0x40CAAC,A0 ; bge */
        do {
            put16(m, a0, rd16(m, a0 - 8u));        /* 0x144740 move.w (-8,A0),(A0) */
            put16(m, a0 + 0x02u, rd16(m, a0 - 6u)); /* 0x144744 */
            put16(m, a0 + 0x04u, rd16(m, a0 - 4u)); /* 0x14474A */
            put8(m, a0 + 0x06u, rd8(m, a0 - 2u));  /* 0x144750 move.b (-2,A0),(6,A0) */
            put8(m, a0 + 0x07u, rd8(m, a0 - 1u));  /* 0x144756 move.b (-1,A0),(7,A0) */
            a0 -= 8u;                              /* 0x14475C lea (-8,A0),A0 */
        } while ((int32_t)a0 > (int32_t)rd32(m, F3_A5 + 0x3320u)); /* 0x144760 bgt */
        put16(m, a0 + 0x02u, 0x00A0u);             /* 0x144766 */
        put16(m, a0 + 0x04u, 0x0080u);             /* 0x14476C */
        put8(m, a0 + 0x06u, 0u);                   /* 0x144772 sf (6,A0) */
        put8(m, a0 + 0x07u, 0u);                   /* 0x144776 sf (7,A0) */
    }

    r->a[0] = a0;
    return 0;                                      /* 0x14477A rts */
}
