#include "obj_12F29E.h"
#include "../objects/obj_pool.h"

/* ROM 0x006422 */
static uint16_t sub_006422(const f3mem_t *m, int16_t d0_in, int16_t d1_in)
{
    uint16_t d0 = (uint16_t)d0_in;
    uint16_t d1 = (uint16_t)d1_in;
    uint16_t d2 = 0u;

    if ((int16_t)d0 < 0) {
        d2 |= 2u;
        d0 = (uint16_t)(0u - d0);
    }
    if ((int16_t)d1 < 0) {
        d2 |= 4u;
        d1 = (uint16_t)(0u - d1);
    }

    uint16_t v = ((int16_t)d1 > (int16_t)d0) ? d1 : d0;
    uint16_t d3;
    if (v == 0u) {
        d3 = 32u;
    } else {
        d3 = 16u;
        while (!(v & 0x8000u)) {
            v = (uint16_t)(v << 1);
            d3++;
        }
    }

    d3 = (uint16_t)(d3 - 0x1Au);
    if ((int16_t)d3 < 0) {
        d3 = (uint16_t)(0u - d3);
        d0 = (uint16_t)(d0 >> d3);
        d1 = (uint16_t)(d1 >> d3);
    }

    if (d2 != 0u && d2 != 6u) {
        uint16_t tmp = d0;
        d0 = d1;
        d1 = tmp;
    }

    uint32_t a0 = 0x00026E40u + (uint32_t)(int32_t)(int16_t)(uint16_t)(d1 << 6);
    uint16_t b = rd8(m, a0 + (uint32_t)(int32_t)(int16_t)d0);
    return (uint16_t)((b + rd16(m, 0x00006492u + (uint32_t)(int32_t)(int16_t)d2)) & 0xFFu);
}

/* ROM 0x12F29E: drift toward the screen origin, pushing the object at A5+0x32AA */
int Obj_12F29E_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {
        Obj_Free(m, obj);
        return 0;
    }
    if (rd8(m, F3_A5 + 0x32AEu) != 0u && rd8(m, obj + 0x22u) == 0u) {
        int16_t s = (int16_t)(rd16(m, obj + 0x42u) + 3u);
        wr16(m, obj + 0x42u, (uint16_t)s);
        if (s >= 0xA0)
            wr8(m, obj + 0x22u, 0xFFu);   /* st */
    } else {
        int16_t s = (int16_t)(rd16(m, obj + 0x42u) - 3u);
        wr16(m, obj + 0x42u, (uint16_t)s);
        if (s <= 0) {
            wr8(m, obj + 0x22u, 0x00u);   /* sf */
            wr16(m, obj + 0x42u, 0u);
            uint16_t d0w = (uint16_t)(0u - (uint16_t)(rd32(m, obj + 0x14u) >> 8));
            uint16_t d1w = (uint16_t)(0u - (uint16_t)(rd32(m, obj + 0x18u) >> 8));
            wr16(m, obj + 0x20u, sub_006422(m, (int16_t)d0w, (int16_t)d1w));
            uint8_t r = (uint8_t)((Rng_NextMem(m) & 0x0Fu) - 8u);
            wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) + r));
        }
    }

    /* 0x12F30C */
    uint32_t a1 = rd32(m, rd32(m, F3_A5 + 0x32AAu) + 0x48u);
    uint32_t i = (uint32_t)((int32_t)(int16_t)rd16(m, obj + 0x20u) * 2);
    int16_t r = (int16_t)rd16(m, obj + 0x42u);
    int32_t d1 = ((int32_t)r * (int32_t)(int16_t)rd16(m, 0x00027E40u + 0x80u + i)) >> 8;
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (uint32_t)d1);
    wr32(m, a1 + 0x56u, rd32(m, a1 + 0x56u) + (uint32_t)d1);
    int32_t d2 = ((int32_t)r * (int32_t)(int16_t)rd16(m, 0x00027E40u + i)) >> 8;
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (uint32_t)d2);
    wr32(m, a1 + 0x5Au, rd32(m, a1 + 0x5Au) + (uint32_t)d2);
    wr32(m, obj + 0x00u, 0x0012F29Eu);
    return 0;
}
