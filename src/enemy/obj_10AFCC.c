#include "obj_10AFCC.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0D0678: script pointer + first frame */
static void sub_0D0678(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr32(m, obj + 0x48u, a1);
    wr16(m, obj + 0x4Cu, rd16(m, a1));
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D0690 */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, t);
    if (t != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0 < 0) {
        uint16_t back = rd16(m, a1);
        if (back == 0u)
            return;
        a1 -= (uint32_t)(int32_t)(int16_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
    } else {
        wr16(m, obj + 0x4Cu, d0);
    }
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x10AFCC: part attached to the parent (+0x08) while it is in state 4 */
int Obj_10AFCC_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    uint8_t st = rd8(m, a1 + 0x06u);
    if (st == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if (st == 4u) {
        if (rd8(m, obj + 0x06u) == 0u) {
            wr8(m, obj + 0x06u, 1u);
            uint32_t a2 = 0x0010B05Eu + (uint32_t)((int32_t)(int16_t)rd16(m, a1 + 0x68u) * 8);
            sub_0D0678(m, obj, rd32(m, a2));
            a2 += 4u;
            wr16(m, obj + 0x50u, rd16(m, a2));
            a2 += 2u;
            wr16(m, obj + 0x54u, rd16(m, a2));
            a1 = rd32(m, obj + 0x08u);
        }
        /* 0x10B010 */
        wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x50u)));
        wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x54u)));
        sub_0D0690(m, obj);
        Coll_AddHazard(m, obj);
        Obj_DrawSprite(m, obj);
        uint32_t h = rd32(m, obj + 0x68u);
        wr16(m, obj + 0x2Cu, rd16(m, h));
        wr16(m, obj + 0x2Eu, rd16(m, h + 2u));
        wr16(m, obj + 0x30u, rd16(m, h + 4u));
        wr16(m, obj + 0x32u, rd16(m, h + 6u));
    }
    /* 0x10B052 */
    Obj_DrawSprite(m, rd32(m, obj + 0x08u));
    return 0;
}
