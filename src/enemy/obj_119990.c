#include "obj_119990.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x119990 */
int Obj_119990_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) == 0u) {
        wr32(m, obj + 0x00u, 0x00119990u);
        return 0;
    }

    uint16_t d = 0x0096u;
    wr16(m, obj + 0x06u, d);
    if (d == 0u) {
        wr32(m, obj + 0x14u, 0x01400000u);
        wr32(m, obj + 0x18u, 0xFFE80000u);
        wr16(m, obj + 0x06u, 0x016Cu);
        wr32(m, obj + 0x00u, 0x001199CCu);
        return 0;
    }

    wr32(m, obj + 0x00u, 0x001199A8u);
    return 0;
}
