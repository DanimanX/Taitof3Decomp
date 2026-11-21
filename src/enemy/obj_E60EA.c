#include "obj_E60EA.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0D0690: advance object animation script */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
{
    uint16_t timer = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, timer);
    if (timer != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0w = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0w < 0) {
        uint16_t off = rd16(m, a1);
        if (off == 0u)
            return;
        a1 = (uint32_t)((int32_t)a1 - (int32_t)(int16_t)off);
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
    } else {
        wr16(m, obj + 0x4Cu, d0w);
    }
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0E60EA */
int Obj_E60EA_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = rd32(m, obj + 0x50u) - rd32(m, F3_A5 + 0x4224u);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + d0);

    d0 = rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d0);

    uint16_t d0w = rd16(m, obj + 0x68u);
    if (d0w != 0u) {
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0w));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d0w));
    }

    sub_0D0690(m, obj);
    if (rd16(m, obj + 0x4Cu) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    Obj_DrawSprite(m, obj);
    return 0;
}
