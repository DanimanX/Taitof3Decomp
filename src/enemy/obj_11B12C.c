#include "obj_11B12C.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x0011B09A: integrate the velocity offsets, draw the sprite while it is
 * on screen, then flip the offsets back off the position. */
static void Obj_11B12C_MoveDraw(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0;
    uint32_t a1;

    d0 = (uint16_t)(rd16(m, obj + 0x50u) + rd16(m, obj + 0x54u) + rd16(m, 0x40B0D2u));
    wr16(m, obj + 0x14u, d0);
    d0 = (uint16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x56u) + rd16(m, 0x40B0D6u));
    wr16(m, obj + 0x18u, d0);

    if (!((int16_t)rd16(m, obj + 0x18u) >= 0xE8 ||
          (int16_t)rd16(m, obj + 0x14u) >= 0x140)) {
        a1 = rd32(m, obj + 0x10u);
        d0 = (uint16_t)(rd16(m, a1) << 4);
        d0 = (uint16_t)(d0 + rd16(m, obj + 0x18u));
        if ((int16_t)d0 > 0) {
            d0 = (uint16_t)(rd16(m, a1 + 2u) << 4);
            d0 = (uint16_t)(d0 + rd16(m, obj + 0x14u));
            if ((int16_t)d0 > 0)
                Obj_DrawSprite(m, obj); /* ROM 0x001282 */
        }
    }

    d0 = rd16(m, obj + 0x54u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
    d0 = rd16(m, obj + 0x56u);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d0));
}

/* ROM 0x0011B124: re-arm the tick vector with oneself. */
static void Obj_11B12C_SetVector(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x0011B12Cu);
}

int Obj_11B12C_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x8u);

    if (rd8(m, a1 + 0x59u) == 0) {
        Obj_11B12C_MoveDraw(m, obj);
        Obj_11B12C_SetVector(m, obj);
        return 0;
    }

    Obj_Free(m, obj); /* ROM 0x0105EC */
    return 0;
}
