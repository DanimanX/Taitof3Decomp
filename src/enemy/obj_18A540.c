#include "obj_18A540.h"

/* ROM 0x18A540 */
int Obj_18A540_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x18A540 */
    wr32(m, obj + 0x64u, 0x00000000u);

    /* 0x18A548: the flow falls through to LAB_0018a548 here (Ghidra also lists
     * 0x18A548 as a separate entry); one write, not a call to the helper. */
    wr32(m, obj + 0x00u, 0x0018A550u);
    return 0;
}
