/* ROM 0x005A3A: upload the character/font tile block. Copies 0x800 longs from
 * ROM 0x1120A to character RAM 0x61E000, swapping the two halves of each long
 * on the way (the ROM holds them little-endian). Every register is saved and
 * restored by the movem pair, so the register file is unchanged at rts. */
#include "fn_auto.h"

int Fn_005A3A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = 0x0001120Au;                       /* 0x005A3E lea (0x1120a).l,A0 */
    uint32_t a1 = 0x0061E000u;                       /* 0x005A48 lea (0x61e000).l,A1 */
    uint16_t d0w = 0x07FFu;                          /* 0x005A44 move.w #0x7ff,D0w */

    do {
        uint32_t d1 = rd32(m, a0);                   /* 0x005A4E move.l (A0)+,D1 */
        a0 += 4u;
        d1 = (d1 << 16) | (d1 >> 16);                /* 0x005A50 swap D1 */
        wr32(m, a1, d1);                             /* 0x005A52 move.l D1,(A1)+ */
        a1 += 4u;
    } while (d0w-- != 0u);                           /* 0x005A54 dbf D0w (0x800 longs) */

    (void)r;                                         /* saved/restored by movem */
    return 0;
}
