#include "obj_117D06.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x117D06 */
int Obj_117D06_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) != 0u) {
        /* unimplemented: ROM 0x117D70 */
        return -1;
    }

    uint32_t a1 = rd32(m, F3_A5 + 0x906u);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x906u, a1 + 4u);

    Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - 0x00006000u);

    uint8_t sum = (uint8_t)(rd8(m, obj + 0x21u) + rd8(m, obj + 0x22u));
    wr8(m, obj + 0x21u, sum);
    if (sum == 0u) {
        wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + 1u));
    }

    a1 = rd32(m, obj + 0x08u);
    Obj_Animate(m, obj, a1);
    Obj_DrawSprite(m, obj);

    if ((int16_t)rd16(m, obj + 0x14u) < -0x64) {
        wr16(m, F3_A5 + 0x88Au, (uint16_t)(rd16(m, F3_A5 + 0x88Au) + 0xB4u));
        Obj_Free(m, obj);
        return 0;
    }

    wr32(m, obj + 0x00u, 0x00117D06u);
    return 0;
}
