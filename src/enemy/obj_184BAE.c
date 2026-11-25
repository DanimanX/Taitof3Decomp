/* ROM 0x184BAE: one-shot fuse, the two-frame variant of 0x184A20. Counts
 * obj+6 down (the capture enters with +6 = 2, so a single call falls to 1),
 * and at zero calls the object factory 0x17B3AE with record 0x186972, group 2
 * of 3, and then spawns a second object from record 0x186908. */
#include "obj_184BAE.h"

/* ROM 0x184BAE */
int Obj_184BAE_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x184BAE: subq.w #1,(0x6,A0) ; bgt.b 0x184BAA */
    uint16_t d = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, d);
    if ((int16_t)d > 0)
        return 0;                       /* 0x184BAA: re-enter at the top */

    /* 0x184BB4: pea 0x186972 ; move.w #2,-(SP) ; move.w #3,-(SP) ;
     * jsr 0x17B3AE -- a spawner that takes two stack words and a record and
     * rewrites the object; no translation in this tree. */
    return -1;
}
