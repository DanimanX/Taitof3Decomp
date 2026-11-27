/* ROM 0x12B204: chained-part initialiser (A0 = object). Publishes the object
 * at A5+0x3292 (0x40B292), runs the segment/link set-up 0x12B386 (its own
 * translation Fn_12B386, which is a bsr with A0 intact, so D0/A1 show at rts),
 * restores the 1.0 damage/speed longs +0x28 and +0x24, sets bits 1, 3 and 4 of
 * +0x34.b and re-arms the object with 0x12B236. */
#include "fn_auto.h"

int Fn_12B204(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr32(m, F3_A5 + 0x3292u, obj);                   /* 0x12B204 move.l A0,(0x3292,A5) */

    Fn_12B386(m, r);                                 /* 0x12B208 bsr.w 0x12B386 */

    wr32(m, obj + 0x28u, 0x00010000u);               /* 0x12B20C */
    wr32(m, obj + 0x24u, 0x00010000u);               /* 0x12B214 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u)); /* 0x12B21C bset.b #1 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x08u)); /* 0x12B222 bset.b #3 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x10u)); /* 0x12B228 bset.b #4 */
    wr32(m, obj + 0x00u, 0x0012B236u);               /* 0x12B22E */
    return 0;
}
