#include "obj_1860AA.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_anim.h"
#include "../video/sprite.h"

/* ROM 0x1860AA: attach to parent +0x08 with offset (+0x16,+0x1A), animate, draw, and fire when A5+0x4B52 bit7 is set. */
int Obj_1860AA_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int16_t)rd16(m, F3_A5 + 0x4B38u) == -1) {
        Obj_Free(m, obj);
        return 0;
    }

    uint32_t parent = rd32(m, obj + 0x08u);
    int16_t x = (int16_t)(rd16(m, parent + 0x50u) + rd16(m, obj + 0x16u));
    int16_t y = (int16_t)(rd16(m, parent + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, (uint16_t)x);
    wr16(m, obj + 0x54u, (uint16_t)y);
    wr16(m, obj + 0x14u, (uint16_t)x);
    wr16(m, obj + 0x18u, (uint16_t)y);
    wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));

    uint16_t anim_idx = rd16(m, obj + 0x40u);
    if (Obj_AnimTick(m, obj, anim_idx) < 0)
        return -1;

    uint32_t spr = rd32(m, obj + 0x10u);
    int16_t h = (int16_t)rd16(m, spr);
    int16_t w = (int16_t)rd16(m, spr + 2u);
    wr16(m, obj + 0x14u, (uint16_t)(x - (w << 3)));
    wr16(m, obj + 0x18u, (uint16_t)(y - (h << 3)));
    Obj_DrawSprite(m, obj);

    uint8_t idx = rd8(m, obj + 0x22u);
    if ((rd8(m, F3_A5 + 0x4B52u) & 0x80u) != 0u) {
        uint32_t entry = 0x0018616Eu + (uint32_t)idx * 12u;
        uint32_t anim_ptr = rd32(m, entry);
        uint16_t new_anim = rd16(m, anim_ptr);
        anim_ptr += 2u;
        wr16(m, obj + 0x40u, new_anim);
        wr32(m, obj + 0x3Au, anim_ptr);
        if (Obj_AnimInit(m, obj, new_anim) < 0)
            return -1;
        uint32_t arg = rd32(m, entry + 8u);
        (void)Obj_Spawn(m, obj, 0x0017D66Eu, 9u, 2u, arg);
    }

    wr32(m, obj + 0x00u, 0x001860AAu);
    return 0;
}
