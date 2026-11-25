/* ROM 0x184A20: one-shot fuse. Counts obj+6 down and, while it is still
 * positive, restarts the same tick from the entry's top (branch back to
 * 0x184A14, so the count falls only by one per call). At zero it calls the
 * generic block spawner 0x1847E8 and then the object factory 0x17B3AE, which
 * installs the next behaviour (0x184D6C) and retires the caller.

 * The two generics are reached after the object is fully rewritten: both read
 * a parameter record from A1 and rewrite the object header, and 0x17B3AE
 * dispatches through the frame engine in RAM, so the routine is translated
 * through the countdown only and reports the spawn path as undecompiled. */
#include "obj_184A20.h"

/* ROM 0x184A20 */
int Obj_184A20_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x184A20: subq.w #1,(0x6,A0) ; bgt.b 0x184A14 */
    uint16_t d = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, d);
    if ((int16_t)d > 0)
        return 0;                       /* 0x184A14: re-enter at the top */

    /* 0x184A26: jsr 0x1847E8 ; jsr 0x17B3AE -- both rewrite the object from a
     * record at A1 and the second dispatches through a RAM table. */
    return -1;
}
