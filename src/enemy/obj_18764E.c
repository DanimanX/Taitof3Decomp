#include "obj_18764E.h"

/* ROM 0x18764E: start a 10-frame delay (the beq at 0x187654 is never taken) */
int Obj_18764E_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x06u, 0x000Au);
    wr32(m, obj + 0x00u, 0x0018765Eu);   /* 0x187656 */
    return 0;
}
