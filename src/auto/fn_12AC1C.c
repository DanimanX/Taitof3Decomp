/* ROM 0x12AC1C: chained-part spawn (A0 = object). Clears the descriptor at
 * +0x0C before running the node/link set-up 0x12B386, then fixes the speed
 * word +0x42 = 0xC0, the two 1.0 damage longs +0x24/+0x28, flags +0x34 bits
 * 1/3/4, the sprite +0x10 = 0x42750, clears +0x50/+0x52/+0x3A and arms
 * 0x12AC6C. D0 (= 0 from 0x12B386's chain walk) and A1 (the chain tail) are
 * left as 0x12B386 leaves them; A0 is untouched. */
#include "fn_auto.h"

int Fn_12AC1C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr32(m, obj + 0x0Cu, 0u);                     /* ROM 0x12AC1C clr.l (0xC,A0) */

    Fn_12B386(m, r);                              /* ROM 0x12AC20 bsr.w 0x12B386 */

    wr16(m, obj + 0x42u, 0x00C0u);                /* ROM 0x12AC24 */
    wr32(m, obj + 0x28u, 0x00010000u);            /* ROM 0x12AC2A */
    wr32(m, obj + 0x24u, 0x00010000u);            /* ROM 0x12AC32 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u)); /* ROM 0x12AC3A bset.b #1 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x08u)); /* ROM 0x12AC40 bset.b #3 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x10u)); /* ROM 0x12AC46 bset.b #4 */
    wr32(m, obj + 0x10u, 0x00042750u);            /* ROM 0x12AC4C */
    wr16(m, obj + 0x50u, 0u);                     /* ROM 0x12AC54 */
    wr16(m, obj + 0x52u, 0u);                     /* ROM 0x12AC5A */
    wr32(m, obj + 0x3Au, 0u);                     /* ROM 0x12AC60 clr.l (0x3A,A0) */
    wr32(m, obj, 0x0012AC6Cu);                    /* ROM 0x12AC64 */
    return 0;                                     /* ROM 0x12AC6A rts */
}
