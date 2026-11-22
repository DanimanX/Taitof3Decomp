#include "obj_11949A.h"
#include "obj_1194C2.h"

/* ROM 0x119590: draw the 1-char string at +0x22 at text cell (+0x14, +0x18), colour +0x1C */
void Text_TypeChar(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x10u, a1);
    uint16_t x = rd16(m, obj + 0x14u), y = rd16(m, obj + 0x18u);
    uint16_t d0 = (uint16_t)((uint16_t)(x + x) + (uint16_t)(y << 7));
    Text_DrawString(m, obj + 0x22u, 0x0061C000u + (uint32_t)(int32_t)(int16_t)d0, rd16(m, obj + 0x1Cu));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 1u));
}

/* ROM 0x11949A: type out the string at +0x10, one char every 3 frames */
int Obj_11949A_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj + 0x00u, 0x0011949Au);   /* 0x119492 */
        return 0;
    }
    /* 0x1194A0 */
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint8_t c = rd8(m, a1);
    a1++;
    wr8(m, obj + 0x22u, c);
    if (c == 0u) {
        wr16(m, obj + 0x06u, 0x003Cu);       /* 0x1194B2 */
        wr32(m, obj + 0x00u, 0x001194C2u);
        return 0;
    }
    Text_TypeChar(m, obj, a1);
    wr16(m, obj + 0x06u, 0x0003u);           /* 0x11948A */
    wr32(m, obj + 0x00u, 0x0011949Au);
    return 0;
}
