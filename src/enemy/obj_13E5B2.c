#include "obj_13E5B2.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x0CBD24 */
static uint16_t sub_CBD24(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t d0;

    for (;;) {
        d0 = rd16(m, obj + 0x3Au);
        uint32_t entry = a1 + (uint32_t)(int32_t)(int16_t)d0;
        int16_t dur = (int16_t)rd16(m, entry);
        if (dur <= 0)
            return d0;
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u);
        wr16(m, obj + 0x3Eu, cnt);
        if ((int16_t)cnt <= dur) {
            wr32(m, obj + 0x10u, rd32(m, entry + 2u));
            return d0;
        }
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x3Au, (uint16_t)(d0 + 6u));
    }
}

/* ROM 0x13E5B2 */
int Obj_13E5B2_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    uint16_t d0 = sub_CBD24(m, obj, a1);

    if ((int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)d0) < 0) {
        Obj_Free(m, obj);
        return 0;
    }

    wr16(m, obj + 0x14u, rd16(m, obj + 0x4Cu));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x50u));

    uint32_t sprdef = rd32(m, obj + 0x10u);
    uint16_t dw = (uint16_t)(rd16(m, sprdef) << 3);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - dw));
    uint16_t dh = (uint16_t)(rd16(m, sprdef + 2u) << 3);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - dh));

    wr32(m, obj + 0x4Cu, rd32(m, obj + 0x4Cu) + rd32(m, obj + 0x44u));
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + rd32(m, obj + 0x48u));
    wr32(m, obj + 0x4Cu, rd32(m, obj + 0x4Cu) + 0x00010000u);
    wr32(m, obj + 0x4Cu, rd32(m, obj + 0x4Cu) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) - rd32(m, F3_A5 + 0x4228u));

    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x0013E5B2u);
    return 0;
}
