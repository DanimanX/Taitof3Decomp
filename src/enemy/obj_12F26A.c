#include "obj_12F26A.h"

/* ROM 0x12F26A: move.w #0xA sets Z=0, so the beq to 0x12F280 is never taken */
int Obj_12F26A_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x06u, 0x000Au);
    wr32(m, obj + 0x00u, 0x0012F27Au);
    return 0;
}
