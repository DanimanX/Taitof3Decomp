#include "obj_12D1C6.h"

/* ROM 0x12D1BE: hold in this state */
static void sub_12D1BE(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0012D1C6u);
}

/* ROM 0x12D1C6: short hold, then drop the boss scroll step to 0 and chain */
int Obj_12D1C6_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
    if ((int16_t)rd16(m, obj + 0x06u) > 0) {
        /* 0x12D1CA: bgt 0x12D1BE */
        sub_12D1BE(m, obj);
        return 0;
    }

    /* 0x12D1CC */
    wr16(m, F3_A5 + 0x32A4u, 0u);
    wr16(m, F3_A5 + 0x32A8u, 3u);

    /* 0x12D1D8 */
    wr32(m, obj + 0x00u, 0x0012D1E0u);
    return 0;
}
