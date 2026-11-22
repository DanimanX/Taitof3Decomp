#include "obj_119422.h"
#include "obj_11949A.h"

/* ROM 0x119422: type the stage name, then a trailing ' on stages 0x15 / 0x1B */
int Obj_119422_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj + 0x00u, 0x00119422u);   /* 0x11941A */
        return 0;
    }
    /* 0x119428 */
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint8_t c = rd8(m, a1);
    a1++;
    wr8(m, obj + 0x22u, c);
    if (c != 0u) {
        Text_TypeChar(m, obj, a1);
        wr16(m, obj + 0x06u, 0x0003u);       /* 0x119412 */
        wr32(m, obj + 0x00u, 0x00119422u);
        return 0;
    }
    /* 0x11943A */
    uint16_t st = rd16(m, F3_A5 + 0x32DCu);
    if (st == 0x0015u || st == 0x001Bu) {
        wr8(m, obj + 0x22u, 0x27u);
        Text_TypeChar(m, obj, a1);
    }
    wr16(m, obj + 0x06u, 0x0018u);           /* 0x11945C */
    wr32(m, obj + 0x00u, 0x0011946Cu);
    return 0;
}
