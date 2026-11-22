#include "obj_119D06.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x119D06 */
int Obj_119D06_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x0Cu));
    Obj_Animate(m, obj, 0x00119D6Eu);        /* lea (0x5e,PC) */
    Obj_DrawSprite(m, obj);
    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u) {
        uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
        wr16(m, obj + 0x06u, t);
        if ((int16_t)t <= 0) {
            wr32(m, obj + 0x00u, 0x00119D36u);   /* 0x119D2E */
            return 0;
        }
    }
    wr32(m, obj + 0x00u, 0x00119D06u);       /* 0x119CFE */
    return 0;
}
