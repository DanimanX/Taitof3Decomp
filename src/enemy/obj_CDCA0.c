#include "obj_CDCA0.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"
#include "../player/player_move.h"

/* ROM 0x0CCC08: apply the spawn argument (+0x0C) offsets and angle,
 * place at the parent's position */
static void sub_0CCC08(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) + (uint16_t)(int16_t)(int8_t)(d0 >> 24)));
    wr16(m, obj + 0x52u, (uint16_t)(rd16(m, obj + 0x52u) + (uint16_t)(int16_t)(int8_t)(d0 >> 16)));
    wr16(m, obj + 0x20u, (uint16_t)((d0 >> 8) & 0xFFu));
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x34u, 0u);
    wr32(m, obj + 0x28u, 0x00010000u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x50u)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x52u)));
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
}

/* ROM 0x0CBD24 */
static uint16_t sub_0CBD24(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    for (;;) {
        uint16_t d0 = rd16(m, obj + 0x3Au);
        uint32_t entry = a1 + (uint32_t)(int32_t)(int16_t)d0;
        int16_t dur = (int16_t)rd16(m, entry);
        if (dur <= 0)
            return d0;
        uint16_t d1 = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u);
        wr16(m, obj + 0x3Eu, d1);
        if ((int16_t)d1 <= dur) {
            wr32(m, obj + 0x10u, rd32(m, entry + 2u));
            return d0;
        }
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x3Au, (uint16_t)(d0 + 6u));
    }
}

/* ROM 0x0CDCA0: directional spark; animation by angle octant */
int Obj_CDCA0_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x50u, 0xFFF0u);
    wr16(m, obj + 0x52u, 0xFFF0u);
    sub_0CCC08(m, obj);
    uint8_t b = (uint8_t)(rd16(m, obj + 0x20u) + 7u);
    wr32(m, obj + 0x58u, rd32(m, 0x000CDD22u + (uint32_t)((b >> 4) & 7u) * 4u));
    wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, 0x000CC10Eu + ((b >> 6) & 3u)) ^ 3u));
    wr16(m, obj + 0x42u, 0x0030u);
    Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));
    wr8(m, obj + 0x1Du, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    /* 0x0CDCF2 */
    uint32_t a1 = rd32(m, obj + 0x58u);
    uint16_t d0 = sub_0CBD24(m, obj, a1);
    if ((int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)d0) < 0) {
        Obj_Free(m, obj);
        return 0;
    }
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x000CDD12u);
    return 0;
}
