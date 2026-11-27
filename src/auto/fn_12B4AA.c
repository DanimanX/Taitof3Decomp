/* ROM 0x12B4AA: object reset/init (A0 = object). Clears the collision words
 * +0x34/+0x1E/+0x1C, the +0x4F byte, sets both HP longs +0x24/+0x28 to
 * 0x00010000, raises flags +0x34 bits 1/3/4, points +0x3A at 0x12C842 and
 * arms 0x12B4EC. No registers are modified. */
#include "fn_auto.h"

int Fn_12B4AA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr16(m, obj + 0x34u, 0u);                     /* ROM 0x12B4AA clr.w (0x34,A0) */
    wr16(m, obj + 0x1Eu, 0u);                     /* ROM 0x12B4AE clr.w (0x1E,A0) */
    wr16(m, obj + 0x1Cu, 0u);                     /* ROM 0x12B4B2 clr.w (0x1C,A0) */
    wr8(m, obj + 0x4Fu, 0u);                      /* ROM 0x12B4B6 sf (0x4F,A0) */
    wr32(m, obj + 0x28u, 0x00010000u);            /* ROM 0x12B4BA */
    wr32(m, obj + 0x24u, 0x00010000u);            /* ROM 0x12B4C2 */
    wr8(m, obj + 0x34u,                          /* ROM 0x12B4CA bset.b #1 */
        (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
    wr8(m, obj + 0x34u,                          /* ROM 0x12B4D0 bset.b #3 */
        (uint8_t)(rd8(m, obj + 0x34u) | 0x08u));
    wr8(m, obj + 0x34u,                          /* ROM 0x12B4D6 bset.b #4 */
        (uint8_t)(rd8(m, obj + 0x34u) | 0x10u));
    wr32(m, obj + 0x3Au, 0x0012C842u);            /* ROM 0x12B4DC */
    wr32(m, obj, 0x0012B4ECu);                    /* ROM 0x12B4E4 */
    return 0;                                     /* ROM 0x12B4EA rts */
}
