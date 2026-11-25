#include "obj_191014.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x17BD1A: boss shake offset table */
static void sub_17BD1A(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x02u) != 0u)
        return;
    uint8_t d0 = (uint8_t)((rd16(m, F3_A5 + 0x0878u) >> 3) & 0x07u);
    d0 = (uint8_t)((d0 + rd8(m, obj + 0x22u)) & 0x07u);
    uint32_t tbl = 0x0017BD48u + (uint32_t)d0 * 4u;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, tbl)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, tbl + 2u)));
}

/* ROM 0x191014: boss part tick */
int Obj_191014_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd16(m, F3_A5 + 0x4B38u) == 0xFFFFu) {
        Obj_Free(m, obj);
        return 0;
    }
    uint32_t a4 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x50u, rd16(m, a4 + 0x50u));
    wr16(m, obj + 0x54u, rd16(m, a4 + 0x54u));
    wr16(m, obj + 0x20u, rd16(m, a4 + 0x20u));
    if (rd8(m, obj + 0x20u) == 0u) {
        if ((rd8(m, obj + 0x23u) & 0x01u) == 0u) {
            uint8_t d0 = rd8(m, obj + 0x22u);
            int fire = (d0 == 0x0Au) ||
                       ((rd8(m, F3_A5 + 0x4B3Bu) & 0x20u) == 0u &&
                        rd8(m, F3_A5 + 0x4B8Eu) == d0);
            if (fire) {
                uint8_t target = (uint8_t)((d0 - 6u) << 5);
                uint8_t cur = (uint8_t)((rd16(m, F3_A5 + 0x0878u) & 0x7Fu) + 1u);
                if (cur == target) {
                    uint32_t a2 = rd32(m, obj + 0x0Cu);
                    uint32_t a1 = rd32(m, a2 + 4u);
                    uint16_t idx = rd16(m, a1); a1 += 2u;
                    wr16(m, obj + 0x40u, idx);
                    wr32(m, obj + 0x3Au, a1);
                    if (Obj_AnimInit(m, obj, idx) < 0)
                        return -1;
                    wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) | 0x01u));
                    if (d0 != 0x0Au)
                        wr8(m, F3_A5 + 0x4B8Eu, (uint8_t)(d0 + 1u));
                }
            }
        }
    } else {
        uint32_t a2 = rd32(m, obj + 0x0Cu);
        uint32_t a1 = rd32(m, a2);
        uint16_t idx = rd16(m, a1); a1 += 2u;
        wr16(m, obj + 0x40u, idx);
        wr32(m, obj + 0x3Au, a1);
        if (Obj_AnimInit(m, obj, idx) < 0)
            return -1;
        wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) & ~0x01u));
        wr8(m, F3_A5 + 0x4B8Eu, 7u);
    }

    uint32_t a1;
    if (rd8(m, obj + 0x22u) == 0x0Au && rd16(m, a4 + 0x40u) == 5u) {
        uint32_t d0 = rd32(m, a4 + 0x10u);
        uint32_t a2 = rd32(m, obj + 0x0Cu);
        a1 = rd32(m, a2 + 0x0Eu);
        for (int i = 0; i < 4; i++) {
            if (rd32(m, a1) == d0)
                break;
            if (i < 3)
                a1 += 6u;
        }
        a1 += 4u;
    } else {
        uint32_t a2 = rd32(m, obj + 0x0Cu);
        a1 = rd32(m, a2 + 8u);
        uint8_t d0 = (uint8_t)(rd8(m, obj + 0x20u) + rd8(m, a2 + 0x0Cu));
        uint16_t d1 = (uint16_t)rd8(m, a2 + 0x0Du);
        uint16_t quot = (uint16_t)((uint16_t)d0 / d1);
        a1 += (uint32_t)quot * 2u;
    }

    int16_t dx = (int16_t)(int8_t)rd8(m, a1);
    int16_t dy = (int16_t)(int8_t)rd8(m, a1 + 1u);
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) + (uint16_t)dx));
    wr16(m, obj + 0x54u, (uint16_t)(rd16(m, obj + 0x54u) + (uint16_t)dy));
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;
    uint32_t spr = rd32(m, obj + 0x10u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - (uint16_t)(rd16(m, spr + 2u) << 3)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - (uint16_t)(rd16(m, spr) << 3)));
    sub_17BD1A(m, obj);
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x00191014u);
    return 0;
}
