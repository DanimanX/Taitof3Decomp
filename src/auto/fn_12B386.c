/* ROM 0x12B386: initialize chained tentacle/segment node and link to tail (A0 = object).
 * Sets +0x20=0x4000, loads +0x42/+0x3A from (+0x0C), assigns segment index
 * +0x22 from parent (+0x08) and increments parent +0x22, appends self to the
 * +0x48 doubly-linked list starting at parent, and clears state/hit fields. */
#include "fn_auto.h"

int Fn_12B386(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr16(m, obj + 0x20u, 0x4000u);                        /* 0x12B386 */
    uint32_t desc = rd32(m, obj + 0x0Cu);                 /* 0x12B38C */
    wr16(m, obj + 0x42u, rd16(m, desc));                  /* 0x12B390 */
    wr32(m, obj + 0x3Au, rd32(m, desc + 2u));             /* 0x12B394 */

    uint32_t a1 = rd32(m, obj + 0x08u);                   /* 0x12B398 */
    wr16(m, obj + 0x22u, rd16(m, a1 + 0x22u));            /* 0x12B39C */
    wr16(m, a1 + 0x22u, (uint16_t)(rd16(m, a1 + 0x22u) + 1u)); /* 0x12B3A2 */

    uint32_t d0 = 0u;                                     /* 0x12B3A6 */
    wr32(m, obj + 0x44u, d0);                             /* 0x12B3A8 */
    wr32(m, obj + 0x48u, d0);                             /* 0x12B3AC */

    while ((d0 = rd32(m, a1 + 0x48u)) != 0u)              /* 0x12B3B0..0x12B3BA */
        a1 = d0;

    wr32(m, obj + 0x44u, a1);                             /* 0x12B3BC */
    wr32(m, a1 + 0x48u, obj);                             /* 0x12B3C0 */
    wr32(m, obj + 0x36u, 0u);                             /* 0x12B3C4 */
    wr16(m, obj + 0x34u, 0u);                             /* 0x12B3C8 */
    wr16(m, obj + 0x1Eu, 0u);                             /* 0x12B3CC */
    wr16(m, obj + 0x1Cu, 0u);                             /* 0x12B3D0 */
    wr8(m, obj + 0x4Fu, 0u);                              /* 0x12B3D4 */
    wr16(m, obj + 0x54u, 0u);                             /* 0x12B3D8 */
    wr16(m, obj + 0x56u, 0u);                             /* 0x12B3DC */

    r->d[0] = d0;
    r->a[1] = a1;
    return 0;
}
