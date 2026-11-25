#include "obj_17C360.h"
#include "../objects/obj_script10.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x17C50A: push the current position/sprite into the 8-entry trail ring */
static void sub_17C50A(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x66u);
    uint16_t d0 = (uint16_t)((rd16(m, obj + 0x6Au) + 1u) & 7u);
    wr16(m, obj + 0x6Au, d0);
    uint32_t e = a1 + (uint32_t)d0 * 8u;
    wr16(m, e + 0u, rd16(m, obj + 0x50u));
    wr16(m, e + 2u, rd16(m, obj + 0x54u));
    wr32(m, e + 4u, rd32(m, obj + 0x10u));
}

/* ROM 0x17C45E: draw the trail behind the object */
static void sub_17C45E(const f3mem_t *m, uint32_t obj)
{
    uint16_t d2 = rd16(m, obj + 0x16u);
    uint16_t d3 = rd16(m, obj + 0x1Au);
    uint16_t d1 = (uint16_t)(d2 << 1);
    uint16_t d4 = (uint16_t)(((uint16_t)rd8(m, obj + 0x1Eu) * d1) >> 8);
    d4 = (uint16_t)(d4 >> 1);
    wr16(m, obj + 0x58u, (uint16_t)(d2 - d4));
    wr16(m, obj + 0x5Au, (uint16_t)(d3 - d4));

    int16_t n = (int16_t)rd16(m, obj + 0x22u);
    do {
        uint32_t a1 = rd32(m, obj + 0x66u);
        uint16_t i = (uint16_t)((rd16(m, obj + 0x6Au) - (uint16_t)n) & 7u);
        a1 += (uint32_t)i * 8u;
        wr16(m, obj + 0x14u, rd16(m, a1));
        wr16(m, obj + 0x18u, rd16(m, a1 + 2u));
        wr32(m, obj + 0x10u, rd32(m, a1 + 4u));
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, obj + 0x58u)));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, obj + 0x5Au)));
        wr8(m, obj + 0x1Du, (uint8_t)(((uint8_t)n << 1) + 0xA0u));
        Obj_DrawSprite(m, obj);
    } while (--n >= 0);
}

/* ROM 0x17C360: live-projectile tick with trail */
int Obj_17C360_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Cu) & 0x10u) != 0u)
        wr32(m, obj + 0x24u, 0u);

    if (rd8(m, obj + 0x1Eu) == 0xF0u) {
        Obj_Free(m, rd32(m, obj + 0x62u));
        Obj_Free(m, obj);
        return 0;
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x01u) != 0u)
        wr32(m, obj + 0x24u, 0u);

    if ((int32_t)rd32(m, obj + 0x24u) > 0) {
        uint32_t a1 = rd32(m, F3_A5 + 0x0906u);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x0906u, a1 + 4u);
    } else {
        wr8(m, obj + 0x1Fu, (uint8_t)(rd8(m, obj + 0x1Fu) + 0x10u));
        wr8(m, obj + 0x1Eu, (uint8_t)(rd8(m, obj + 0x1Eu) + 0x10u));
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1 = (uint16_t)(rd16(m, a1) << 4);
        uint16_t d0 = (uint16_t)(((uint16_t)rd8(m, obj + 0x1Fu) * d1) >> 8);
        d0 = (uint16_t)(d0 >> 1);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0));
    }

    if (Obj_Script10(m, obj) < 0)
        return -1;

    if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
        Obj_Free(m, rd32(m, obj + 0x62u));
        Obj_Free(m, obj);
        return 0;
    }
    if (rd32(m, obj + 0x44u) != 0x0017C344u) {
        int outside = 0;
        int16_t x = (int16_t)rd16(m, obj + 0x50u);
        int16_t lo = (int16_t)rd16(m, 0x001849F8u), hi = (int16_t)rd16(m, 0x001849FAu);
        if (!(x >= lo && x <= hi)) {
            outside = 1;
        } else {
            int16_t y = (int16_t)rd16(m, obj + 0x54u);
            int16_t lo2 = (int16_t)rd16(m, 0x001849FCu), hi2 = (int16_t)rd16(m, 0x001849FEu);
            if (!(y >= lo2 && y <= hi2))
                outside = 1;
        }
        if (outside) {
            wr32(m, obj + 0x44u, 0x0017C344u);
            wr16(m, obj + 0x48u, rd16(m, 0x0017C348u));
            wr16(m, obj + 0x4Au, 0u);
        }
    }

    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    sub_17C50A(m, obj);
    sub_17C45E(m, obj);
    wr32(m, obj + 0x00u, 0x0017C360u);
    return 0;
}
