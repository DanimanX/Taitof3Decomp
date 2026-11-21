#include "obj_DC674.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0D0690 */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
{
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, cnt);
    if (cnt != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0 < 0) {
        int16_t back = (int16_t)rd16(m, a1);
        if (back == 0)
            return;
        a1 -= (uint32_t)(int32_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
        a1 += 2u;
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        return;
    }

    wr16(m, obj + 0x4Cu, d0);
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0DC674 */
int Obj_DC674_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x50u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u)));
    sub_0D0690(m, obj);
    if (rd16(m, obj + 0x4Cu) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    Obj_DrawSprite(m, obj);
    return 0;
}
