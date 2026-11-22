#include "obj_12D0AE.h"

/* ROM 0x12D0AE: the beq at 0x12D0BA is never taken */
int Obj_12D0AE_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj + 0x00u, 0x0012D0AEu);   /* 0x12D0A6 */
        return 0;
    }
    wr16(m, obj + 0x06u, 0x0096u);
    wr32(m, obj + 0x00u, 0x0012D0C4u);       /* 0x12D0BC */
    return 0;
}
