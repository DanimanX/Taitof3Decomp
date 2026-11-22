#include "obj_18A540.h"

/* ROM 0x18A548 */
static void sub_18A548(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0018A550u);
}

/* ROM 0x18A540 */
int Obj_18A540_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x18A540 */
    wr32(m, obj + 0x64u, 0x00000000u);

    /* 0x18A548: the bundle marks 18A540 -> 18A548 as an unconditional
     * fall-through (its "beq" is the second word of the #imm32 above being
     * read as an opcode), so the first step runs on the next tick. */
    sub_18A548(m, obj);
    return 0;
}
