#include "obj_11423E.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x0C6632: terrain bit test at (x, y) against 0x40891C bitmap. */
static uint8_t sub_0C6632(const f3mem_t *m, uint16_t d0w, uint16_t d1w)
{
    if (rd8(m, F3_A5 + 0x1D1Cu) != 0u)
        return 0u;
    uint32_t tx = ((uint16_t)(d0w + rd16(m, F3_A5 + 0x0918u)) >> 3) & 0x3FFu;
    uint32_t ty = ((uint16_t)(d1w + rd16(m, F3_A5 + 0x091Au)) >> 3) & 0x3Fu;
    uint32_t bit = ty * 0x280u + tx;
    uint8_t byte = rd8(m, 0x0040891Cu + (bit >> 3));
    return ((byte & (0x80u >> (bit & 7u))) != 0u) ? 0xFFu : 0x00u;
}

/* ROM 0x0CBE12: advance +0x14/+0x18 by +0x42 speed along angle. */
static void sub_0CBE12(const f3mem_t *m, uint32_t obj, int16_t angle)
{
    int16_t spd = (int16_t)rd16(m, obj + 0x42u);
    int16_t cos_v = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)(int32_t)angle * 2u);
    int16_t sin_v = (int16_t)rd16(m, 0x00027E40u + (uint32_t)(int32_t)angle * 2u);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (uint32_t)((int32_t)spd * (int32_t)cos_v));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (uint32_t)((int32_t)spd * (int32_t)sin_v));
}

/* ROM 0x11423E */
int Obj_11423E_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t parent = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, parent + 0x14u) + 7u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, parent + 0x18u) + 7u));
    uint32_t pc = rd32(m, parent + 0x0Cu);
    uint8_t lo = (uint8_t)(((uint8_t)pc << 3) + 0xE0u);
    wr16(m, obj + 0x20u, (uint16_t)((pc & 0xFF00u) | lo));
    wr16(m, obj + 0x2Cu, 6u);
    wr16(m, obj + 0x2Eu, 9u);
    wr16(m, obj + 0x30u, 6u);
    wr16(m, obj + 0x32u, 9u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x1Eu, 0xC0C0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x24u, 0x00010000u);
    wr32(m, obj + 0x28u, 0x00010000u);
    wr16(m, obj + 0x42u, 0x000Au);
    if (rd16(m, obj + 0x0Cu) == 0u) {
        uint8_t b = rd8(m, obj + 0x21u);
        b = (uint8_t)(-((int8_t)(uint8_t)(b - 0x40u)) + 0x40u);
        wr8(m, obj + 0x21u, b);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 3u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 7u));
    }
    wr16(m, obj + 0x06u, 0x0018u);

    /* 0x1142CE */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0)
        goto die_0cd3d6;
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        goto die_0cd3d6;
    if (sub_0C6632(m, (uint16_t)(rd16(m, obj + 0x14u) + 8u),
                      (uint16_t)(rd16(m, obj + 0x18u) + 8u)) != 0u)
        goto die_0cd3d6;
    {
        uint32_t gp = rd32(m, rd32(m, obj + 0x08u) + 0x08u);
        uint8_t fl = rd8(m, gp + 0x50u);
        if ((fl & 0x08u) == 0u) {
            if ((fl & 0x01u) != 0u || (fl & 0x10u) != 0u)
                goto die_0cd3d6;
        }
        if ((fl & 0x20u) != 0u)
            goto die_0cd3d6;
    }

    Obj_Animate(m, obj, 0x001143E4u);
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }

    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
    if ((int16_t)rd16(m, obj + 0x06u) > 0) {
        wr16(m, obj + 0x1Eu, (uint16_t)(rd16(m, obj + 0x1Eu) - 0x0808u));
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - 0x00006000u);
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - 0x00006000u);
    } else {
        if (rd16(m, obj + 0x0Cu) == 0u) {
            Coll_AddHazard(m, obj);
        } else {
            uint32_t lp = rd32(m, F3_A5 + 0x0902u);
            wr32(m, lp, obj);
            wr32(m, F3_A5 + 0x0902u, lp + 4u);
        }
        sub_0CBE12(m, obj, (int16_t)rd16(m, obj + 0x20u));
    }

    Obj_DrawSprite(m, obj);
    {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        int16_t y = (int16_t)rd16(m, obj + 0x18u);
        if (x < (int16_t)rd16(m, 0x000CC112u) || x > (int16_t)rd16(m, 0x000CC114u) ||
            y < (int16_t)rd16(m, 0x000CC116u) || y > (int16_t)rd16(m, 0x000CC118u)) {
            Obj_Free(m, obj);
            return 0;
        }
    }
    wr32(m, obj + 0x00u, 0x001143C8u);
    return 0;

die_0cd3d6:
    {
        uint32_t a1 = rd32(m, 0x000CD444u + 1u * 4u);
        uint16_t snd = rd16(m, a1);
        a1 += 2u;
        if (snd != 0u)
            Sound_PlayCmd(m, Sound_PreCmdHook(m, (int16_t)snd));
        wr32(m, obj + 0x0Cu, a1);
        wr16(m, obj + 0x3Au, 0u);
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x1Eu, 0u);
        wr16(m, obj + 0x1Cu, 0u);
        wr32(m, obj + 0x00u, 0x000CD412u);
        return 0;
    }
}
