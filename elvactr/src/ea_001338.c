/* ROM 0x001338: read a long from A5-0x785E, multiply by 7, add 5, store back, swap D0 halves */
#include "ea_auto.h"

int Ea_001338(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t d0 = r->d[0];
    uint32_t addr = a5 - 0x785Eu;
    uint32_t sp = r->a[7];

    d0 = rd32(m, addr);                          /* 0x001338 move.l -$785e(a5), d0 */
    d0 *= 7;                                     /* 0x00133C muls.l #$7, d0 */
    d0 += 5;                                     /* 0x001344 addq.l #$5, d0 */
    wr32(m, addr, d0);                           /* 0x001346 move.l d0, -$785e(a5) */
    d0 = (d0 << 16) | (d0 >> 16);                /* 0x00134A swap d0 */

    r->d[0] = d0;                                /* 0x00134C rts */
    r->a[7] = sp + 4u;
    return 0;
}
