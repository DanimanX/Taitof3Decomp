#include "obj_128E44.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x128E3C: timer still positive: stay on this entry */
static void sub_128E3C(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x00128E44u);
}

/* ROM 0x128E44: advance the frame-list cursor (+0xC), draw, count down +0x6;
 * when it runs out restart the list, centre the sprite and switch to 0x128E8C */
int Obj_128E44_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x0Cu, a1);
    Obj_DrawSprite(m, obj);

    int16_t n = (int16_t)(uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        sub_128E3C(m, obj);
        return 0;
    }
    /* 0x128E5C */
    wr32(m, obj + 0x0Cu, rd32(m, F3_A5 + 0x1D28u));
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Cu, rd16(m, obj + 0x20u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 8u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u));
    wr16(m, obj + 0x22u, 0u);
    wr16(m, obj + 0x06u, 0x003Cu);
    wr32(m, obj + 0x00u, 0x00128E8Cu);
    return 0;
}
