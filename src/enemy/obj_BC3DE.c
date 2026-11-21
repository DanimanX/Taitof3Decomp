#include "obj_BC3DE.h"

/* ROM 0x0BC3DE */
int Obj_BC3DE_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0)
        wr32(m, obj + 0x00u, 0x000BC3DEu);
    else
        wr32(m, obj + 0x00u, 0x000BC3ECu);
    return 0;
}
