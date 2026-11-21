#include "obj_C9978.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x0C9978 */
int Obj_C9978_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1;
    uint16_t d0w;
    uint32_t d0;

    /* ROM 0x0C9978 */
    wr8(m, obj + 0x22u, 0x00u);
    wr32(m, obj + 0x44u, 0x000C999Eu);
    wr32(m, obj + 0x24u, 0x00010000u);
    wr32(m, obj + 0x28u, 0x00010000u);
    wr16(m, obj + 0x1Eu, 0x0000u);

    /* ROM 0x0C9BCA */
    a1 = rd32(m, obj + 0x08u);
    {
        int16_t old_cnt = (int16_t)rd16(m, a1 + 0x5Au);
        wr16(m, a1 + 0x5Au, (uint16_t)((uint16_t)old_cnt - 1u));
        if (old_cnt <= 0)
            goto lab_0c9d68;
    }
    if ((rd8(m, a1 + 0x65u) & 0x01u) != 0u)
        goto lab_0c9d68;

    wr32(m, obj + 0x54u, 0x00000000u);
    wr32(m, obj + 0x50u, rd32(m, obj + 0x28u));
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x000Fu));

    a1 = rd32(m, obj + 0x44u);
    d0w = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0w));

    d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1);
    a1 += 1u;
    wr16(m, obj + 0x2Cu, d0w);

    d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1);
    a1 += 1u;
    wr16(m, obj + 0x30u, d0w);

    d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1);
    a1 += 1u;
    wr16(m, obj + 0x2Eu, d0w);

    d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1);
    a1 += 1u;
    wr16(m, obj + 0x32u, d0w);

    wr32(m, obj + 0x44u, a1);
    wr16(m, obj + 0x1Cu, 0x0000u);
    wr16(m, obj + 0x34u, 0x0000u);
    wr16(m, obj + 0x3Au, 0x0000u);
    wr16(m, obj + 0x3Eu, 0x0000u);

    a1 = rd32(m, obj + 0x0Cu);
    d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1);
    a1 += 1u;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0w));

    d0w = (uint16_t)(int16_t)(int8_t)rd8(m, a1);
    a1 += 1u;
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0w));

    d0 = (uint32_t)(int32_t)(int16_t)rd16(m, a1) << 8;
    a1 += 2u;
    wr32(m, obj + 0x48u, d0);

    d0 = (uint32_t)(int32_t)(int16_t)rd16(m, a1) << 8;
    a1 += 2u;
    wr32(m, obj + 0x4Cu, d0);

    /* ROM 0x0C9C62 */
    wr32(m, obj, 0x000C9C6Au);
    return 0;

lab_0c9d68:
    /* ROM 0x0C9D68 */
    a1 = rd32(m, obj + 0x08u);
    wr16(m, a1 + 0x5Au, (uint16_t)(rd16(m, a1 + 0x5Au) + 1u));
    Obj_Free(m, obj);
    return 0;
}
