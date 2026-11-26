#include "obj_CBF04.h"
#include "../objects/obj_pool.h"

/* ROM 0x0CBF06: spawn +0x22 children of routine +0x36 (group 5, count 2,
 * arg = running index +0x54), stopping at the first failed spawn. The last
 * child gets +0x0C = our +0x4E. +0x56/+0x58 = the number spawned; with
 * any, wait in 0x0CBF5E for them, else free in 0x0CBF6C. */
static int spawn_0CBF06(const f3mem_t *m, uint32_t obj)
{
    uint16_t d1 = rd16(m, obj + 0x22u);
    uint32_t a1 = rd32(m, obj + 0x36u);
    wr16(m, obj + 0x54u, 0u);
    d1 = (uint16_t)(d1 - 1u);                           /* subq.w ; dbra */
    for (;;) {
        uint32_t d2 = rd16(m, obj + 0x54u);
        uint32_t d0 = Obj_Spawn(m, obj, a1, 5u, 2u, d2);
        if ((int32_t)d0 < 0)
            goto done;                                  /* bmi 0x0CBF46 */
        wr16(m, obj + 0x54u, (uint16_t)(rd16(m, obj + 0x54u) + 1u));
        if (d1-- == 0u) {                               /* dbra falls out */
            wr8(m, d0 + 0x0Cu, rd8(m, obj + 0x4Eu));
            break;
        }
    }
done:;
    uint16_t n = rd16(m, obj + 0x54u);
    wr16(m, obj + 0x58u, n);
    wr16(m, obj + 0x56u, n);
    wr32(m, obj + 0x00u, n != 0u ? 0x000CBF5Eu : 0x000CBF6Cu);
    return 0;
}

/* ROM 0x0CBF04: bra 0x0CBEF2 */
int Obj_CBF04_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t v = rd32(m, F3_A5 + 0x1D28u);
    wr32(m, obj + 0x50u, v);                            /* 0x0CBEF2 */
    if (v != 0u)
        return spawn_0CBF06(m, obj);
    wr32(m, obj + 0x00u, 0x000CBF04u);                  /* 0x0CBEFC */
    return 0;
}

int Obj_CBEEA(const f3mem_t *m, uint32_t obj, uint16_t d1w, uint32_t a1)
{
    wr16(m, obj + 0x22u, d1w);
    wr32(m, obj + 0x36u, a1);
    return Obj_CBF04_Tick(m, obj);
}
