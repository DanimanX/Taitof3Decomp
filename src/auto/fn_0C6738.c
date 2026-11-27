/* ROM 0x0C6738: expand the RLE terrain map 0x16AD1E[stage word at A5+0x32DC]
 * into Work RAM 0x40891C with the run-length filler 0x16A2F6. Every register
 * is saved (movem 0x0C6738/0x0C6768) and restored, so the caller's registers
 * survive; A2 = dst+0x1400 is set but 0x16A2F6 never reads it. */
#include "fn_auto.h"
#include "../common/fn_leaf1.h"

int Fn_0C6738(const f3mem_t *m, f3regs_t *r)
{
    uint32_t d0 = r->d[0], d7 = r->d[7];
    uint32_t a0 = r->a[0], a1 = r->a[1], a2 = r->a[2];
    int16_t idx = (int16_t)rd16(m, F3_A5 + 0x32DCu);                  /* ROM 0x0C6742 */
    uint32_t src = rd32(m, 0x0016AD1Eu + (uint32_t)(int32_t)idx * 4u); /* ROM 0x0C673C..0x0C6746 */

    r->a[0] = 0x0040891Cu;                    /* ROM 0x0C674A: lea 0x40891C,A1 -> A0 (0x0C6756) */
    r->a[1] = src;                            /* ROM 0x0C6758: movea.l (SP)+,A1 */
    r->a[2] = 0x0040891Cu + 0x1400u;          /* ROM 0x0C675A: lea (0x1400,A0),A2 */
    Leaf_16A2F6(m, r);                        /* ROM 0x0C675E: jsr 0x0016A2F6 */

    r->d[0] = d0; r->d[7] = d7;               /* ROM 0x0C6768: movem.l (SP)+,{D0 D7 A0 A1} */
    r->a[0] = a0; r->a[1] = a1; r->a[2] = a2;
    return 0;                                 /* ROM 0x0C676C rts */
}
