#include "obj_E66F8.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../audio/sound_cmd.h"

/* ROM 0x0CFA86: frame record {colour.w, sprdef.l, hitbox.l} */
static void sub_0CFA86(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    wr32(m, obj + 0x10u, rd32(m, a1 + 2u));
    wr32(m, obj + 0x68u, rd32(m, a1 + 6u));
}

/* ROM 0x0E66F8: horizontal beam of 20 sprites (1 + 19) following the parent; frames
 * 0xE6862[+6] every 4 frames, then dies */
int Obj_E66F8_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t p = rd32(m, obj + 0x08u);
    if (rd8(m, p + 0x06u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    wr32(m, obj + 0x14u, rd32(m, p + 0x14u));
    uint16_t d0 = rd8(m, obj + 0x22u) ? (uint16_t)-0x0D : 0x016Du;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0));
    wr32(m, obj + 0x18u, rd32(m, p + 0x18u));
    uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, t);
    if (t == 0u) {
        uint8_t k = (uint8_t)(rd8(m, obj + 0x06u) - 1u);
        wr8(m, obj + 0x06u, k);
        if ((int8_t)k < 0) {
            Obj_StartDying(m, obj);
            return 0;
        }
        sub_0CFA86(m, obj, rd32(m, 0x000E6862u + (uint32_t)k * 4u));
        wr8(m, obj + 0x23u, 4u);
    }
    Coll_AddHazard(m, obj);
    Obj_DrawSprite(m, obj);
    for (int i = 0; i < 19; i++) {
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x10u));
        Obj_DrawSprite(m, obj);
    }
    return 0;
}
