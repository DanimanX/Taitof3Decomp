/* ROM 0x12B5BA: initialize child segment from parent (A0 = object).
 * Copies +0x44 script and +0x42->+0x22 from parent (+0x08), sets hitboxes,
 * HP/dmg = 0x10000, flags (+0x34 bits 1,3,4), clears anim state, and
 * transitions to 0x12B624. */
#include "fn_auto.h"

int Fn_12B5BA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t parent = rd32(m, obj + 0x08u);               /* 0x12B5BA */

    wr32(m, obj + 0x44u, rd32(m, parent + 0x44u));        /* 0x12B5BE */
    wr16(m, obj + 0x22u, rd16(m, parent + 0x42u));        /* 0x12B5C4 */
    wr16(m, obj + 0x34u, 0u);                             /* 0x12B5CA */
    wr16(m, obj + 0x1Eu, 0u);                             /* 0x12B5CE */
    wr16(m, obj + 0x1Cu, 0u);                             /* 0x12B5D2 */
    wr8(m, obj + 0x4Fu, 0u);                              /* 0x12B5D6 */
    wr16(m, obj + 0x2Cu, 0x0007u);                        /* 0x12B5DA */
    wr16(m, obj + 0x30u, 0x0007u);                        /* 0x12B5E0 */
    wr16(m, obj + 0x2Eu, 0x0019u);                        /* 0x12B5E6 */
    wr16(m, obj + 0x32u, 0x0019u);                        /* 0x12B5EC */
    wr32(m, obj + 0x28u, 0x00010000u);                    /* 0x12B5F2 */
    wr32(m, obj + 0x24u, 0x00010000u);                    /* 0x12B5FA */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u)); /* 0x12B602 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x08u)); /* 0x12B608 */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x10u)); /* 0x12B60E */
    wr16(m, obj + 0x3Au, 0u);                             /* 0x12B614 */
    wr16(m, obj + 0x3Eu, 0u);                             /* 0x12B618 */
    wr32(m, obj, 0x0012B624u);                            /* 0x12B61C */

    r->a[1] = parent;
    return 0;
}
