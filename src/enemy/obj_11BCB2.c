#include "obj_11BCB2.h"
#include "../objects/obj_pool.h"

/* ROM 0x11BCB2 */
int Obj_11BCB2_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x11BCB2: velocity from the parent's direction table (arg +0xC) */
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    if ((int16_t)rd16(m, a1 + 0x18u) >= 0x80) {
        wr16(m, obj + 0x18u, (uint16_t)-0x40);
        wr16(m, obj + 0x20u, 0x4000u);
        wr8(m, obj + 0x3Eu, (uint8_t)-0x04);
    } else {
        wr16(m, obj + 0x18u, 0x0128u);
        wr16(m, obj + 0x20u, (uint16_t)-0x4000);
        wr8(m, obj + 0x3Eu, 0x04u);
    }

    /* 0x11BCE8: five children, then speed and angle spread */
    wr16(m, obj + 0x14u, 0x0140u);
    for (uint16_t d1 = 5u; ; d1--) {
        Obj_Spawn(m, obj, 0x0011BDAAu, 5u, 2u, (uint32_t)d1);
        if (d1 == 0u)
            break;
    }

    wr16(m, obj + 0x22u, 0u);
    uint16_t r = (uint16_t)Rng_NextMem(m);      /* D0.w */
    wr16(m, obj + 0x2Cu, (uint16_t)(r & 0x000Fu));
    wr16(m, obj + 0x2Eu, (uint16_t)(r & 0x000Fu));
    r = (uint16_t)Rng_NextMem(m);
    wr16(m, obj + 0x30u, (uint16_t)(r & 0x000Fu));
    wr16(m, obj + 0x32u, (uint16_t)(r & 0x000Fu));

    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x42u, 0x0008u);

    /* 0x11BD40 */
    wr32(m, obj + 0x00u, 0x0011BD48u);
    return 0;
}
