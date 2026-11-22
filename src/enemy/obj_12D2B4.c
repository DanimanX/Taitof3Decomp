#include "obj_12D2B4.h"

/* ROM 0x12D2AC: hold in this state */
static void sub_12D2AC(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0012D2B4u);
}

/* ROM 0x12D2B4: slide the background, hold for 0x3C ticks then chain */
int Obj_12D2B4_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x12D2B4 */
    wr16(m, F3_A5 + 0x32A2u, (uint16_t)(rd16(m, F3_A5 + 0x32A2u) - 0x00C0u));
    wr16(m, F3_A5 + 0x32A4u, (uint16_t)(rd16(m, F3_A5 + 0x32A4u) + 0x0060u));

    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
    if ((int16_t)rd16(m, obj + 0x06u) > 0) {
        /* 0x12D2C4: bgt 0x12D2AC */
        sub_12D2AC(m, obj);
        return 0;
    }

    /* 0x12D2C6 */
    wr16(m, obj + 0x06u, 0x003Cu);

    /* 0x12D2CC */
    wr32(m, obj + 0x00u, 0x0012D2D4u);
    return 0;
}
