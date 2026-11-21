#include "obj_E02C4.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0E02C4 */
int Obj_E02C4_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0E02C4 */
    uint32_t a1 = rd32(m, obj + 0x08u);
    if (rd8(m, a1 + 0x06u) == 0u)
        wr8(m, obj + 0x06u, 0u);

    /* 0x0E02D4 */
    if (rd8(m, obj + 0x06u) != 0u) {
        uint8_t cnt = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, cnt);
        if (cnt == 0u) {
            a1 = rd32(m, obj + 0x08u);
            if (rd8(m, a1 + 0x06u) == 5u) {
                uint32_t spawned = Obj_Spawn(m, obj, 0x000DF7EAu, 9u, 2u, 0u);
                if ((int32_t)spawned >= 0) {
                    a1 = spawned;
                    uint32_t a2 = rd32(m, obj + 0x54u);
                    wr8(m, a1 + 0x06u, 1u);
                    wr32(m, obj + 0x54u, a1);
                    wr32(m, a1 + 0x50u, obj);
                    wr32(m, a1 + 0x54u, a2);
                    if ((int32_t)a2 >= 0)
                        wr32(m, a2 + 0x50u, a1);
                }
            }
            /* 0x0E0334 */
            wr8(m, obj + 0x23u, 7u);
        }
    }

    /* 0x0E033A */
    uint32_t d0 = rd32(m, obj + 0x54u);
    if ((int32_t)d0 < 0) {
        if (rd8(m, obj + 0x06u) == 0u)
            Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x0E034E */
    do {
        uint32_t cur = d0;
        if (rd8(m, cur + 0x06u) == 0u) {
            uint32_t scroll_y = rd32(m, F3_A5 + 0x4228u);
            wr32(m, cur + 0x18u, rd32(m, cur + 0x18u) - scroll_y);
            Obj_DrawSprite(m, cur);
        }
        /* 0x0E0366 */
        d0 = rd32(m, cur + 0x54u);
    } while ((int32_t)d0 >= 0);

    return 0;
}
