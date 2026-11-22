#include "obj_1194C2.h"
#include "../objects/obj_pool.h"

/* ROM 0x005666: text string -> text RAM as {colour*2, char} byte pairs */
uint32_t Text_DrawString(const f3mem_t *m, uint32_t src, uint32_t dst, uint16_t col)
{
    uint8_t d7 = (uint8_t)(col + col);
    for (;;) {
        uint8_t c = rd8(m, src++);
        if (c == 0u)
            break;
        wr8(m, dst++, d7);
        wr8(m, dst++, c);
    }
    return dst;
}

/* ROM 0x1194C2 */
int Obj_1194C2_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj + 0x00u, 0x001194C2u);   /* 0x1194BA */
        return 0;
    }
    Text_DrawString(m, 0x00119518u, 0x0061C88Cu, 7u);   /* pea (0x44,PC) */
    Text_DrawString(m, 0x00119518u, 0x0061C998u, 7u);   /* pea (0x2a,PC) */
    Obj_Free(m, rd32(m, obj + 0x0Cu));
    Obj_Free(m, obj);
    return 0;
}
