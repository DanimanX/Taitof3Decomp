#include "obj_D5EE8.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* ROM 0x0CBDA4: append obj to active collision/render lists (A5+0x8FE, A5+0x906). */
static void sub_0CBDA4(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);

    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x0906u, a1 + 4u);
}

/* ROM 0x0D60A8: set +0x22 = 0xFF if any player is within X range [0xD615E, 0xD6160]. */
static void sub_0D60A8(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = F3_A5 + 0x1D2Cu;
    int16_t d0 = (int16_t)rd16(m, F3_A5 + 0x1D34u);
    int16_t low = (int16_t)rd16(m, 0x000D6160u);
    int16_t high = (int16_t)rd16(m, 0x000D6162u);
    int16_t obj_x = (int16_t)rd16(m, obj + 0x14u);

    while (--d0 >= 0) {
        uint32_t a2 = rd32(m, a1);
        a1 += 4u;
        int16_t dx = (int16_t)((int16_t)rd16(m, a2 + 0x14u) - obj_x);
        if (dx >= low && dx <= high) {
            wr8(m, obj + 0x22u, 0xFFu);
            return;
        }
    }
}

/* ROM 0x0D60D0: return 0xFF if any player is within dx [0xD6162, 0xD6164] and dy < 0xD6168. */
static uint8_t sub_0D60D0(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = F3_A5 + 0x1D2Cu;
    int16_t d1 = (int16_t)rd16(m, F3_A5 + 0x1D34u);
    int16_t low = (int16_t)rd16(m, 0x000D6164u);
    int16_t high = (int16_t)rd16(m, 0x000D6166u);
    int16_t dy_limit = (int16_t)rd16(m, 0x000D6168u);
    int16_t obj_x = (int16_t)rd16(m, obj + 0x14u);
    int16_t obj_y = (int16_t)rd16(m, obj + 0x18u);

    while (--d1 >= 0) {
        uint32_t a2 = rd32(m, a1);
        a1 += 4u;
        int16_t dx = (int16_t)((int16_t)rd16(m, a2 + 0x14u) - obj_x);
        if (dx < low || dx > high)
            continue;
        int16_t dy = (int16_t)((int16_t)rd16(m, a2 + 0x18u) - obj_y);
        if (dy >= dy_limit)
            continue;
        return 0xFFu;
    }
    return 0x00u;
}

/* ROM 0x0D5EE8 */
int Obj_D5EE8_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        /* 0x0D6048 */
        uint32_t a1 = Obj_0CBDDE(m, obj);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
        goto lab_d6058;
    }

    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u)
        goto lab_d6058;

    if (rd8(m, obj + 0x22u) == 0u) {
        if ((int16_t)rd16(m, obj + 0x14u) <= (int16_t)rd16(m, 0x000CC112u)) {
            Obj_Free(m, obj);                                       /* 0x0D609A */
            return 0;
        }
        wr32(m, obj + 0x10u, 0x000641ECu);
        sub_0D60A8(m, obj);
    } else {
        /* 0x0D5F20 */
        Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, obj + 0x20u) >> 8));
        if ((int16_t)rd16(m, obj + 0x18u) >= (int16_t)rd16(m, 0x000CC118u)) {
            Obj_Free(m, obj);                                       /* 0x0D609A */
            return 0;
        }
        Obj_Animate(m, obj, 0x000D6114u);
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) + 1u);
        wr16(m, obj + 0x06u, cnt);
        if ((int16_t)cnt >= 0x28) {
            if ((int16_t)cnt >= 0x78 || sub_0D60D0(m, obj) != 0u) {
                /* 0x0D5FA8 */
                Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x0033));
                wr16(m, obj + 0x3Au, 0u);
                wr16(m, obj + 0x3Eu, 0u);
                wr16(m, obj + 0x2Cu, 9u);
                wr16(m, obj + 0x30u, 4u);
                wr16(m, obj + 0x2Eu, 0x46u);
                wr16(m, obj + 0x32u, 0x1Bu);
                wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x18u));
                wr8(m, obj + 0x1Du, 0u);
                wr8(m, obj + 0x35u, 0u);
                wr16(m, obj + 0x06u, 0x2Bu);
                wr32(m, obj + 0x00u, 0x000D5FFEu);                  /* 0x0D5FF6 */
                return 0;
            }
        }
    }

    /* 0x0D5F68 */
    if (rd8(m, F3_A5 + 0x1D58u) & 0x02u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            /* 0x0CBD12 */
            uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }

    /* 0x0D5F8C */
    Obj_SubScrollVel(m, obj);
    sub_0CBDA4(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x000D5EE8u);                              /* 0x0D5EE0 */
    return 0;

lab_d6058:
    Sound_PlayCmd(m, Sound_PreCmdHook(m, (int16_t)(uint16_t)rd8(m, obj + 0x65u)));
    Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, Obj_13E4D0(m, obj, 2u));
    wr32(m, obj + 0x00u, 0x000D609Au);
    return 0;
}
