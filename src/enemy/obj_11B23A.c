#include "obj_11B23A.h"
#include "../objects/obj_pool.h"

/* ROM 0x11B232: timer still positive: stay on this entry */
static void sub_11B232(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0011B23Au);
}

/* ROM 0x11B23A: drag the global scroll (+0x30D2/+0x30D6) from the object's
 * own +0x14/+0x18 (asr.l 6), then free the object after +0x6 ticks */
int Obj_11B23A_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = (uint32_t)((int32_t)rd32(m, obj + 0x14u) >> 6);    /* asr.l #6 */
    wr32(m, F3_A5 + 0x30D2u, rd32(m, F3_A5 + 0x30D2u) - d0);
    d0 = (uint32_t)((int32_t)rd32(m, obj + 0x18u) >> 6);
    wr32(m, F3_A5 + 0x30D6u, rd32(m, F3_A5 + 0x30D6u) - d0);

    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        sub_11B232(m, obj);
        return 0;
    }
    /* 0x11B254: move.l A0,-(SP); jsr 0x0105EC; lea (4,SP),SP */
    Obj_Free(m, obj);
    return 0;
}
