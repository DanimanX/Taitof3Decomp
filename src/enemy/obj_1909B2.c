#include "obj_1909B2.h"

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

/* ROM 0x1909B2: chain root tick -- track the parent, run the kinematics and
 * shift the whole chain by the pivot link's displacement */
int Obj_1909B2_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x190A24: pivot = link number A5+0x4B46 down the +0x66 chain */
    uint32_t a1 = obj;
    for (uint16_t n = rd16(m, F3_A5 + 0x4B46u); n != 0u; n--)
        a1 = rd32(m, a1 + 0x66u);
    wr32(m, F3_A5 + 0x4B84u, a1);

    uint32_t a4 = rd32(m, obj + 0x08u);
    uint16_t d0 = (uint16_t)(rd16(m, a4 + 0x50u) - rd16(m, F3_A5 + 0x4B04u));
    uint16_t d1 = (uint16_t)(rd16(m, a4 + 0x54u) - rd16(m, F3_A5 + 0x4B08u));
    a1 = rd32(m, F3_A5 + 0x4B84u);
    d0 = (uint16_t)(d0 + rd16(m, a1 + 0x50u));
    d1 = (uint16_t)(d1 + rd16(m, a1 + 0x54u));
    wr16(m, F3_A5 + 0x4B8Au, d0);
    wr16(m, F3_A5 + 0x4B8Cu, d1);
    wr16(m, F3_A5 + 0x4B88u, rd16(m, a1 + 0x60u));

    a4 = rd32(m, obj + 0x08u);
    d0 = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
    d1 = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, d0);
    wr16(m, obj + 0x54u, d1);
    wr16(m, obj + 0x14u, d0);
    wr16(m, obj + 0x18u, d1);

    wr16(m, obj + 0x5Eu, rd16(m, 0x0040CB80u));   /* 0x190A3C */
    sub_17BB80(m, obj);
    sub_17BC1E(m, obj);
    sub_17BCB6(m, obj);

    /* 0x190A68: move 12 links by (saved pivot pos - current pivot pos) */
    a1 = rd32(m, F3_A5 + 0x4B84u);
    d0 = (uint16_t)(rd16(m, F3_A5 + 0x4B8Au) - rd16(m, a1 + 0x50u));
    d1 = (uint16_t)(rd16(m, F3_A5 + 0x4B8Cu) - rd16(m, a1 + 0x54u));
    a1 = obj;
    for (int n = 0; n < 12; n++) {
        wr16(m, a1 + 0x50u, (uint16_t)(rd16(m, a1 + 0x50u) + d0));
        wr16(m, a1 + 0x54u, (uint16_t)(rd16(m, a1 + 0x54u) + d1));
        a1 = rd32(m, a1 + 0x66u);
    }
    a4 = rd32(m, obj + 0x08u);
    wr16(m, a4 + 0x50u, rd16(m, obj + 0x50u));
    wr16(m, a4 + 0x54u, rd16(m, obj + 0x54u));
    wr32(m, obj, 0x1909B2u);
    return 0;
}
