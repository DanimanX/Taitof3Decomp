#include "obj_C7DCA.h"

/* ROM 0x0C7DCA */
int Obj_C7DCA_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x000C7DCAu);
        return 0;
    }
    wr8(m, F3_A5 + 0x1D36u, (uint8_t)(rd8(m, F3_A5 + 0x1D36u) | 0x04u));
    wr32(m, obj + 0x00u, 0x000C7DDEu);
    return 0;
}
