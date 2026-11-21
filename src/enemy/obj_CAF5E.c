#include "obj_CAF5E.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0CB14A: place at the screen centre scaled by +0x1E, draw */
static void sub_0CB14A(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x14u, 0x00A0u);
    wr16(m, obj + 0x18u, 0x0074u);
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t d0w = (uint16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x0100u);
    int32_t p = (int32_t)(int16_t)d0w * (int32_t)(int16_t)rd16(m, a1);
    d0w = (uint16_t)((int16_t)(uint16_t)p >> 5);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0w));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0w));
    d0w = (uint16_t)(d0w << 2);
    d0w = (uint16_t)(0u - d0w);
    wr16(m, F3_A5 + 0x1D5Cu, d0w);
    Obj_DrawSprite(m, obj);
}

/* ROM 0x0CAF5E: zoom in */
int Obj_CAF5E_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Animate(m, obj, 0x000CB0F8u);
    sub_0CB14A(m, obj);
    uint16_t z = rd16(m, obj + 0x1Eu);
    wr16(m, obj + 0x1Eu, (uint16_t)(z - 0x1010u));
    if (z > 0x1010u) {                     /* bhi */
        wr32(m, obj + 0x00u, 0x000CAF5Eu);
        return 0;
    }
    wr8(m, F3_A5 + 0x1D58u, (uint8_t)(rd8(m, F3_A5 + 0x1D58u) | 0x02u));
    wr16(m, obj + 0x1Eu, 0x0000u);
    wr16(m, obj + 0x06u, 0x003Cu);
    wr32(m, obj + 0x00u, 0x000CAF90u);
    return 0;
}
