#include "obj_17F812.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"
#include "../video/sprite.h"

/* ROM 0x17F812: follow parent (+0x50/+0x54) via 10-byte script (0x178982),
 * step 0x179A5C animation, zoom-center sprite, draw, and free when script ends. */
int Obj_17F812_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t parent = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x4Cu, rd16(m, parent + 0x50u));
    wr16(m, obj + 0x4Eu, rd16(m, parent + 0x54u));

    if (Obj_Script10(m, obj) < 0)
        return -1;

    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    uint32_t spr = rd32(m, obj + 0x10u);
    uint16_t h8 = (uint16_t)(rd16(m, spr) << 3);
    uint16_t w8 = (uint16_t)(rd16(m, spr + 2u) << 3);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - w8));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - h8));

    wr8(m, obj + 0x1Fu, (uint8_t)(rd8(m, obj + 0x1Fu) + 4u));
    wr8(m, obj + 0x1Eu, (uint8_t)(rd8(m, obj + 0x1Eu) + 4u));

    uint16_t d1 = (uint16_t)(rd16(m, spr) << 4);
    uint16_t d0 = rd8(m, obj + 0x1Fu);
    uint16_t off = (uint16_t)(((uint16_t)((uint32_t)d0 * d1) >> 8) >> 1);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + off));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + off));

    Obj_DrawSprite(m, obj);

    if (rd8(m, obj + 0x1Eu) >= 0xF0u)
        wr16(m, obj + 0x1Eu, (uint16_t)(int16_t)-0x0F10);

    if (rd32(m, obj + 0x44u) != 0xFFFFFFFFu) {
        wr32(m, obj + 0x00u, 0x0017F812u);
    } else {
        Obj_Free(m, obj);
    }
    return 0;
}
