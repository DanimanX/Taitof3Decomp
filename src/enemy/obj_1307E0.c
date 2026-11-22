#include "obj_1307E0.h"
#include "../objects/obj_pool.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x1307E0 */
int Obj_1307E0_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x1307E0 */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) == 0u) {
        uint32_t a1 = rd32(m, obj + 0x08u);
        wr16(m, obj + 0x14u, rd16(m, a1 + 0x14u));
        wr16(m, obj + 0x18u, rd16(m, a1 + 0x18u));
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x08u));
        /* subi.w #-4 == addq.w #4 */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x04u));

        /* 0x130804 */
        Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));
        Obj_DrawSprite(m, obj);

        /* 0x130814 */
        uint16_t t = (uint16_t)(rd16(m, obj + 0x42u) - 0x08u);
        wr16(m, obj + 0x42u, t);
        if ((int16_t)t > 0) {
            wr32(m, obj + 0x00u, 0x001307E0u);
            return 0;
        }
    }

    Obj_Free(m, obj);                               /* 0x13081A */
    return 0;
}
