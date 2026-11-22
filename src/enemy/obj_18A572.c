#include "obj_18A572.h"

/* ROM 0x18A572 */
int Obj_18A572_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x18A572 */
    wr32(m, obj + 0x64u, 0x00080000u);

    /* 0x18A57A: the flow falls through to LAB_0018a57a here (Ghidra also lists
     * 0x18A57A as a separate entry); one write, not a call to the helper. */
    wr32(m, obj + 0x00u, 0x0018A582u);
    return 0;
}
