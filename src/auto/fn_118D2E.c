/* ROM 0x118D2E: object step (A0 = object). Arms +0x3A with 0x118D3A, then
 * pops the next word of the (+0x0C) pointer list into (+0x06) and re-arms
 * the object with 0x118D7C. */
#include "fn_auto.h"

int Fn_118D2E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t a1;

    wr32(m, obj + 0x3Au, 0x00118D3Au);                   /* 0x118D2E */

    /* 0x118D36: bra.w 0x118D66 */
    a1 = rd32(m, obj + 0x0Cu);                           /* 0x118D66 */
    uint16_t w = rd16(m, a1);                            /* 0x118D6A */
    a1 += 2u;
    wr32(m, obj + 0x0Cu, a1);                            /* 0x118D6C */
    wr16(m, obj + 0x06u, w);                             /* 0x118D70 */

    wr32(m, obj + 0x00u, 0x00118D7Cu);                   /* 0x118D74 */
    r->d[0] = (d0 & 0xFFFF0000u) | (uint32_t)w;
    r->a[1] = a1;
    return 0;
}
