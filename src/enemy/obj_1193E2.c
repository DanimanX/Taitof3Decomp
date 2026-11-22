#include "obj_1193E2.h"
#include "obj_11949A.h"

/* ROM 0x1193E2: type the first string, then switch to the stage name */
int Obj_1193E2_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj + 0x00u, 0x001193E2u);   /* 0x1193DA */
        return 0;
    }
    /* 0x1193E8 */
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint8_t c = rd8(m, a1);
    a1++;
    wr8(m, obj + 0x22u, c);
    if (c != 0u) {
        Text_TypeChar(m, obj, a1);
        wr16(m, obj + 0x06u, 0x0003u);       /* 0x1193D2 */
        wr32(m, obj + 0x00u, 0x001193E2u);
        return 0;
    }
    /* 0x1193FA */
    a1 = 0x0013D5C8u + (uint32_t)((int32_t)(int16_t)rd16(m, F3_A5 + 0x32DCu) * 2);
    wr32(m, obj + 0x10u, a1);
    wr16(m, obj + 0x1Cu, 0x0007u);
    wr16(m, obj + 0x06u, 0x0003u);           /* 0x119412 */
    wr32(m, obj + 0x00u, 0x00119422u);
    return 0;
}
