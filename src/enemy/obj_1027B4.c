#include "obj_1027B4.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

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
        d0 = rd16(m, a1);
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)(d0 >> 8));
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        return;
    }

    wr16(m, obj + 0x4Cu, d0);
    d0 = rd16(m, a1);
    a1 += 2u;
    wr8(m, obj + 0x1Cu, (uint8_t)(d0 >> 8));
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x1027B4 */
int Obj_1027B4_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);

    uint32_t v3a = rd32(m, a1 + 0x14u);
    wr32(m, obj + 0x3Au, v3a);
    uint32_t v3e = rd32(m, a1 + 0x18u);
    wr32(m, obj + 0x3Eu, v3e);
    wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));
    uint8_t b = rd8(m, a1 + 0x06u);
    wr8(m, obj + 0x22u, b);
    if (b == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    wr32(m, obj + 0x14u, v3a);
    wr32(m, obj + 0x18u, v3e);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x10u));

    sub_0D0690(m, obj);
    return 0;
}
