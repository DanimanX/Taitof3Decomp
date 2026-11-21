#include "obj_CABAC.h"

/* ROM 0x0CABAC: init */
int Obj_CABAC_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x2Cu, 0x0002u);
    wr16(m, obj + 0x30u, 0x0002u);
    wr16(m, obj + 0x2Eu, 0x000Du);
    wr16(m, obj + 0x32u, 0x000Du);
    wr8(m, obj + 0x22u, 0x01u);
    wr16(m, obj + 0x20u, 0x0000u);
    wr32(m, obj + 0x28u, 0x00008000u);
    wr32(m, obj + 0x24u, 0x00010000u);
    wr8(m, obj + 0x06u, 0x00u);   /* sf */
    wr32(m, obj + 0x00u, 0x000CABF4u);
    return 0;
}
