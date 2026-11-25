#include "obj_17F6C8.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* ROM 0x17F6C8 */
int Obj_17F6C8_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, 0x00409D58u) & 0x01u) == 0u &&
        (rd8(m, F3_A5 - 0x144Cu) & 0x10u) == 0u) {
        uint32_t parent = rd32(m, obj + 0x08u);
        uint16_t x = (uint16_t)(rd16(m, parent + 0x50u) + rd16(m, obj + 0x16u));
        uint16_t y = (uint16_t)(rd16(m, parent + 0x54u) + rd16(m, obj + 0x1Au));
        wr16(m, obj + 0x50u, x);
        wr16(m, obj + 0x54u, y);
        wr16(m, obj + 0x14u, x);
        wr16(m, obj + 0x18u, y);

        if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
            return -1;

        uint32_t spr = rd32(m, obj + 0x10u);
        int16_t dy = (int16_t)((int16_t)rd16(m, spr + 0x00u) << 3);
        int16_t dx = (int16_t)((int16_t)rd16(m, spr + 0x02u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)((int16_t)rd16(m, obj + 0x14u) - dx));
        wr16(m, obj + 0x18u, (uint16_t)((int16_t)rd16(m, obj + 0x18u) - dy));
        Obj_DrawSprite(m, obj);

        int16_t cnt = (int16_t)(rd16(m, obj + 0x64u) - 1u);
        wr16(m, obj + 0x64u, (uint16_t)cnt);
        if (cnt > 0) {
            wr32(m, obj + 0x00u, 0x0017F6C8u);
            return 0;
        }
    }

    int16_t cmd = Sound_PreCmdHook(m, 0x0022);
    Sound_PlayCmdForced(m, cmd);
    wr32(m, obj + 0x00u, 0x0017F74Eu);
    return 0;
}
