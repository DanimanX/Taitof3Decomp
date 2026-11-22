#include "obj_131414.h"
#include "../objects/obj_pool.h"

/* ROM 0x001660: enqueue an effect at A5-0x6750 + (A5-0x5DF0)*8 */
static void sub_001660(const f3mem_t *m, uint32_t a0, uint16_t d0w, uint16_t d1w)
{
    uint16_t d2w = rd16(m, F3_A5 - 0x5DF0u);
    if ((int16_t)d2w >= 0x20)
        return;

    uint32_t a2 = (F3_A5 - 0x6750u) + (uint32_t)(uint16_t)(d2w << 3);
    wr16(m, a2, d1w);
    wr32(m, a2 + 0x02u, a0);
    wr16(m, a2 + 0x06u, d0w);
    wr16(m, F3_A5 - 0x5DEEu, 0x0002u);
    wr16(m, F3_A5 - 0x5DF0u, (uint16_t)(d2w + 1u));
}

/* ROM 0x131414 */
int Obj_131414_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);

    /* 0x131418 */
    if (rd8(m, a1 + 0x59u) != 0u) {
        Obj_Free(m, obj);                           /* 0x131454 */
        return 0;
    }

    /* 0x131420: lea (0x40,PC) at 0x131420 -> 0x131462 */
    a1 = 0x00131462u + (uint32_t)(int32_t)(int16_t)rd16(m, obj + 0x3Au);
    sub_001660(m, a1, 0x5E04u, 0x0008u);            /* 0x131428 */

    /* 0x131440 */
    uint16_t t = (uint16_t)(rd16(m, obj + 0x3Au) + 4u);
    wr16(m, obj + 0x3Au, t);
    if ((int16_t)t < 0x40) {
        wr32(m, obj + 0x00u, 0x00131414u);
        return 0;
    }

    /* 0x13144E */
    wr16(m, obj + 0x3Au, 0u);
    wr32(m, obj + 0x00u, 0x00131414u);
    return 0;
}
