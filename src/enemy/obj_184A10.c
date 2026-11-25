/* ROM 0x184A10: one-shot spawn tick. Seeds obj+6 = 0x0A and hands the object
 * to 0x184A20, which counts +6 down and spawns through the generic
 * 0x1847E8 / 0x17B3AE pair when it reaches zero. */
#include "obj_184A10.h"

/* ROM 0x184A10 */
int Obj_184A10_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x184A10: move.w #0xA,(0x6,A0) */
    wr16(m, obj + 0x06u, 0x000Au);

    /* 0x184A16: beq.w 0x184A26 -- the branch tests the status register left
     * by the caller's previous compare (not part of the captured register
     * set); the captured entry reaches 0x184A18. */

    /* 0x184A18: move.l #0x184A20,(A0) ; rts */
    wr32(m, obj, 0x00184A20u);
    return 0;
}
