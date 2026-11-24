#include "obj_17C240.h"
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

/* ROM 0x17C530: draw the trail behind the object */
static void sub_17C530(const f3mem_t *m, uint32_t obj)
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
        wr16(m, obj + 0x14u, rd16(m, a1)); a1 += 2u;
        wr16(m, obj + 0x18u, rd16(m, a1)); a1 += 2u;
        wr32(m, obj + 0x10u, rd32(m, a1));
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, obj + 0x58u)));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, obj + 0x5Au)));
        Obj_DrawSprite(m, obj);
    } while (--n >= 0);
}

/* ROM 0x17C240: live-projectile body the spawn-wait tails hand over to. */
int Obj_17C240_Tick(const f3mem_t *m, uint32_t obj)
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
        /* 0x17C29A: run the +0x0C callback, then the hazard registration */
        uint32_t a1 = rd32(m, F3_A5 + 0x0906u);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x0906u, a1 + 4u);
    } else {
        wr8(m, obj + 0x1Fu, (uint8_t)(rd8(m, obj + 0x1Fu) + 0x10u));
        wr8(m, obj + 0x1Eu, (uint8_t)(rd8(m, obj + 0x1Eu) + 0x10u));
        {
            uint32_t a1 = rd32(m, obj + 0x10u);
            uint16_t d1 = (uint16_t)(rd16(m, a1) << 4);
            uint16_t d0 = (uint16_t)(((uint16_t)rd8(m, obj + 0x1Fu) * d1) >> 8);
            d0 = (uint16_t)(d0 >> 1);
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0));
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0));
        }
    }

    /* 0x17C2AA: the movement script runs on both paths */
    if (Obj_Script10(m, obj) < 0)
        return -1;

    if (rd32(m, obj + 0x44u) == 0xFFFFFFFFu) {
        Obj_Free(m, rd32(m, obj + 0x62u));
        Obj_Free(m, obj);
        return 0;
    }
    if (rd32(m, obj + 0x44u) != 0x0017C344u) {
        /* 0x17C2CC / 0x17C2DC: bcs is taken when the value is OUTSIDE the
         * (lo,hi) pair, and either miss switches to the 0x17C344 record. */
        int outside = 0;
        int16_t x = (int16_t)rd16(m, obj + 0x50u);
        int16_t lo = (int16_t)rd16(m, 0x184A00u), hi = (int16_t)rd16(m, 0x184A02u);
        if (!(x >= lo && x <= hi))
            outside = 1;
        else {
            int16_t y2 = (int16_t)rd16(m, obj + 0x54u);
            int16_t lo2 = (int16_t)rd16(m, 0x1849FCu), hi2 = (int16_t)rd16(m, 0x1849FEu);
            if (!(y2 >= lo2 && y2 <= hi2))
                outside = 1;
        }
        if (outside) {
            wr32(m, obj + 0x44u, 0x0017C344u);
            wr16(m, obj + 0x48u, rd16(m, 0x0017C348u)); /* record +4 */
            wr16(m, obj + 0x4Au, 0x0000u);
        }
    }

    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

    {
        uint16_t idx = rd16(m, obj + 0x40u);
        if (Obj_AnimTick(m, obj, idx) < 0)
            return -1;
    }
    sub_17C50A(m, obj);
    sub_17C530(m, obj);
    wr32(m, obj + 0x00u, 0x0017C240u);
    return 0;
}
