#include "obj_18A572.h"

/* ROM 0x18A57A */
static void sub_18A57A(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0018A582u);
}

/* ROM 0x18A572 */
int Obj_18A572_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x18A572 */
    wr32(m, obj + 0x64u, 0x00080000u);

    /* 0x18A57A: the bundle marks 18A572 -> 18A57A as an unconditional
     * fall-through (its "beq" is the second word of the #imm32 above being
     * read as an opcode), so the step 0x18A582 runs on the next tick. */
    sub_18A57A(m, obj);
    return 0;
}
