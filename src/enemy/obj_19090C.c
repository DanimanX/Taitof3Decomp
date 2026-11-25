#include "obj_19090C.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x17BB80: update joint angles along the +0x62 chain */
static void sub_17BB80(const f3mem_t *m, uint32_t obj)
{
    int16_t d7 = (int16_t)((int16_t)rd16(m, F3_A5 + 0x4B48u) - (int16_t)rd16(m, F3_A5 + 0x4B46u));
    if (d7 < 0)
        return;
    uint32_t a4 = rd32(m, obj + 0x62u);
    uint16_t d2 = rd16(m, F3_A5 + 0x4B4Au);
    int16_t d1 = (int16_t)rd16(m, F3_A5 + 0x4B4Cu);
    if ((rd8(m, F3_A5 + 0x4B27u) & 0x80u) == 0u)
        d1 = (int16_t)rd16(m, F3_A5 + 0x4B00u) >> (d2 & 63u);
    wr16(m, F3_A5 + 0x4B00u, (uint16_t)d1);
    d1 = 0;
    int8_t dir = (int8_t)rd8(m, F3_A5 + 0x4B4Eu);
    if (dir == 0)
        dir = (int8_t)rd8(m, a4 + 0x60u);
    if (dir < 0)
        d1 = (int16_t)rd16(m, F3_A5 + 0x4B00u);
    else if (dir > 0)
        d1 = (int16_t)(-(int16_t)rd16(m, F3_A5 + 0x4B00u));
    for (;;) {
        uint32_t a3 = rd32(m, a4 + 0x62u);
        d1 = (int16_t)(d1 + (int16_t)rd16(m, a4 + 0x60u));
        int16_t d0 = (int16_t)((int16_t)rd16(m, a3 + 0x60u) - d1);
        if (d0 < 0)
            d0 = (int16_t)(-d0);
        if (d0 <= 0x1000) {
            wr16(m, a4 + 0x60u, (uint16_t)d1);
            if (rd8(m, a4 + 0x60u) == 0xFFu)
                wr16(m, a4 + 0x60u, 0u);
        }
        d1 = (int16_t)((int16_t)rd16(m, a4 + 0x60u) - (int16_t)rd16(m, a3 + 0x60u)) >> (d2 & 63u);
        a4 = a3;
        if (d7 == 0)
            break;
        d7--;
    }
}

/* ROM 0x17BC1E: update joint angles along the +0x66 chain */
static void sub_17BC1E(const f3mem_t *m, uint32_t obj)
{
    int16_t d7 = (int16_t)rd16(m, F3_A5 + 0x4B44u);
    if (d7 < 0)
        return;
    uint32_t a4 = obj;
    uint16_t d2 = rd16(m, F3_A5 + 0x4B4Au);
    int16_t d1 = (int16_t)rd16(m, F3_A5 + 0x4B4Cu);
    if ((rd8(m, F3_A5 + 0x4B27u) & 0x80u) == 0u)
        d1 = (int16_t)rd16(m, F3_A5 + 0x4B00u) >> (d2 & 63u);
    wr16(m, F3_A5 + 0x4B00u, (uint16_t)d1);
    d1 = 0;
    int8_t dir = (int8_t)rd8(m, F3_A5 + 0x4B4Eu);
    if (dir < 0)
        d1 = (int16_t)(-(int16_t)rd16(m, F3_A5 + 0x4B00u));
    else if (dir > 0)
        d1 = (int16_t)rd16(m, F3_A5 + 0x4B00u);
    else {
        int8_t b = (int8_t)rd8(m, a4 + 0x60u);
        if (b < 0)
            d1 = (int16_t)rd16(m, F3_A5 + 0x4B00u);
        else if (b > 0)
            d1 = (int16_t)(-(int16_t)rd16(m, F3_A5 + 0x4B00u));
    }
    for (;;) {
        uint32_t a3 = rd32(m, a4 + 0x66u);
        d1 = (int16_t)(d1 + (int16_t)rd16(m, a4 + 0x60u));
        int16_t d0 = (int16_t)((int16_t)rd16(m, a3 + 0x60u) - d1);
        if (d0 < 0)
            d0 = (int16_t)(-d0);
        if (d0 <= 0x1000) {
            wr16(m, a4 + 0x60u, (uint16_t)d1);
            if (rd8(m, a4 + 0x60u) == 0xFFu)
                wr16(m, a4 + 0x60u, 0u);
        }
        d1 = (int16_t)((int16_t)rd16(m, a4 + 0x60u) - (int16_t)rd16(m, a3 + 0x60u)) >> (d2 & 63u);
        a4 = a3;
        if (d7 == 0)
            break;
        d7--;
    }
}

/* ROM 0x17BCB6: forward kinematics along the +0x66 child chain */
static void sub_17BCB6(const f3mem_t *m, uint32_t obj)
{
    uint16_t d7 = rd16(m, F3_A5 + 0x4B48u);
    uint32_t a3 = obj;
    wr8(m, a3 + 0x20u, (uint8_t)(rd8(m, a3 + 0x60u) + rd8(m, a3 + 0x5Eu)));
    for (;;) {
        uint32_t a4 = a3;
        a3 = rd32(m, a3 + 0x66u);
        uint16_t d2 = (uint8_t)(rd8(m, a4 + 0x60u) + rd8(m, a4 + 0x5Eu) + rd8(m, a4 + 0x5Cu));
        int16_t d3 = (int16_t)rd8(m, a4 + 0x42u);
        int16_t c = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)d2 * 2u);
        int16_t s = (int16_t)rd16(m, 0x00027E40u + (uint32_t)d2 * 2u);
        int32_t x = (int32_t)rd32(m, a4 + 0x50u) + (int32_t)c * (int32_t)d3;
        int32_t y = (int32_t)rd32(m, a4 + 0x54u) + (int32_t)s * (int32_t)d3;
        wr32(m, a3 + 0x50u, (uint32_t)x);
        wr32(m, a3 + 0x54u, (uint32_t)y);
        wr8(m, a3 + 0x20u, (uint8_t)(rd8(m, a3 + 0x60u) + rd8(m, a3 + 0x5Eu)));
        if (d7 == 0u)
            break;
        d7--;
    }
}

/* ROM 0x19090C: spawn the 11-link 0x190AA6 chain (+0x66 down, +0x62 back),
 * seed the root and run the first kinematics pass */
int Obj_19090C_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a3 = obj;
    uint32_t d0 = 0;
    for (uint32_t d1 = 0; d1 < 11u; d1++) {
        d0 = Obj_Spawn(m, obj, 0x190AA6u, 6u, 1u, d1);
        wr32(m, a3 + 0x66u, d0);
        wr32(m, d0 + 0x62u, a3);
        a3 = d0;
    }
    wr32(m, obj + 0x62u, d0);
    Obj_17A7FA(m, obj);
    wr16(m, obj + 0x42u, 0x5A00u);
    wr16(m, obj + 0x5Cu, 0xC600u);
    wr16(m, obj + 0x5Eu, 0u);
    wr16(m, obj + 0x60u, 0u);
    wr16(m, F3_A5 + 0x4B48u, 0x000Au);
    wr16(m, F3_A5 + 0x4B4Au, 2u);
    wr16(m, F3_A5 + 0x4B4Cu, 0x0200u);
    uint32_t a4 = rd32(m, obj + 0x08u);
    uint16_t x = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t y = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, x);
    wr16(m, obj + 0x54u, y);
    wr16(m, obj + 0x14u, x);
    wr16(m, obj + 0x18u, y);
    sub_17BB80(m, obj);
    sub_17BC1E(m, obj);
    sub_17BCB6(m, obj);
    wr32(m, obj, 0x1909B2u);
    return 0;
}
