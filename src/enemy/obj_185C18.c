#include "obj_185C18.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_script10.h"
#include "../video/sprite.h"

/* ROM 0x185C18: script-driven sprite tick; frees self + child (+0x64) when X <= -32. */
int Obj_185C18_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int16_t)rd16(m, obj + 0x50u) <= -32) {
        Obj_Free(m, rd32(m, obj + 0x64u));
        Obj_Free(m, obj);
        return 0;
    }

    if (Obj_Script10(m, obj) < 0)
        return -1;

    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

    uint16_t anim_idx = rd16(m, obj + 0x40u);
    if (Obj_AnimTick(m, obj, anim_idx) < 0)
        return -1;

    uint32_t spr = rd32(m, obj + 0x10u);
    int16_t h = (int16_t)rd16(m, spr);
    int16_t w = (int16_t)rd16(m, spr + 2u);
    wr16(m, obj + 0x14u, (uint16_t)((int16_t)rd16(m, obj + 0x14u) - (w << 3)));
    wr16(m, obj + 0x18u, (uint16_t)((int16_t)rd16(m, obj + 0x18u) - (h << 3)));
    Obj_DrawSprite(m, obj);

    wr32(m, obj + 0x00u, 0x00185C18u);
    return 0;
}
