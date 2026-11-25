#include "obj_124550.h"

/* ROM 0x124550 */
int Obj_124550_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, F3_A5 + 0x30D2u, rd32(m, F3_A5 + 0x30D2u) + 0x00020000u);
    if ((int16_t)rd16(m, F3_A5 + 0x30D2u) < 0x0122) {
        wr32(m, obj, 0x00124550u);
        return 0;
    }
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) & (uint8_t)~0x04u));
    wr16(m, obj + 0x06u, 3u);
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x80u));
    wr16(m, obj + 0x3Eu, 0x001Eu);
    wr32(m, obj, 0x001244A2u);
    return 0;
}
