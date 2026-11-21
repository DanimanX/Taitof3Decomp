#include "obj_102860.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x000D0690 */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
{
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, cnt);
    if (cnt != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u);
    a1 = (uint32_t)(a1 + (int32_t)(int16_t)0x000Cu);
    wr32(m, obj + 0x48u, a1);

    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0 >= 0) {
        wr16(m, obj + 0x4Cu, d0);
        uint16_t w = rd16(m, a1);
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)w);
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        return;
    }

    /* LAB_000d06c0 */
    uint16_t t = rd16(m, a1);
    if (t != 0u) {
        a1 = (uint32_t)(a1 - (int32_t)(int16_t)t);
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        uint16_t w2 = rd16(m, a1);
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)w2);
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
    }
    /* LAB_000d06de: rts */
}

/* ROM 0x102860 */
int Obj_102860_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x3Au, rd32(m, a1 + 0x3Au));
    wr32(m, obj + 0x3Eu, rd32(m, a1 + 0x3Eu));
    wr8(m, obj + 0x1Du, rd8(m, a1 + 0x1Du));
    uint8_t b22 = rd8(m, a1 + 0x22u);
    wr8(m, obj + 0x22u, b22);
    if (b22 == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0048u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0008u));
    sub_0D0690(m, obj);
    return 0;
}
