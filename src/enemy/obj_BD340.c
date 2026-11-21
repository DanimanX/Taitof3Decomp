#include "obj_BD340.h"
/* ROM 0xBD340 */
int Obj_BD340_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BD340 */
    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
    if ((int16_t)rd16(m, obj + 0x06u) > 0) {
        /* 0x0BD338 */
        wr32(m, obj, 0x000BD340u);
        return 0;
    }
    /* 0x0BD346 */
    wr16(m, obj + 0x60u, 0x003Fu);
    /* 0x0BD34C */
    wr32(m, obj, 0x000BD354u);
    return 0;
}
